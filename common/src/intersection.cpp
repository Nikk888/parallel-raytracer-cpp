#include "../include/intersection.hpp"
#include "../include/cylinder.hpp"
#include "../include/hit.hpp"
#include "../include/ray.hpp"
#include "../include/scene.hpp"
#include "../include/sphere.hpp"
#include "../include/vector.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <vector>

namespace render {

  // Small epsilon to avoid floating-point self-intersection issues
  static constexpr double EPS = 1e-3;

  namespace {

    struct AABB {
      Vector3 min;
      Vector3 max;

      [[nodiscard]] bool hit(Ray const & ray, double t_min, double t_max) const noexcept {
        Vector3 const & o = ray.origin();
        Vector3 const & d = ray.direction();

        double inv = 1.0 / d.x;
        double t0  = (min.x - o.x) * inv;
        double t1  = (max.x - o.x) * inv;
        if (inv < 0.0) {
          std::swap(t0, t1);
        }
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min) {
          return false;
        }

        inv = 1.0 / d.y;
        t0  = (min.y - o.y) * inv;
        t1  = (max.y - o.y) * inv;
        if (inv < 0.0) {
          std::swap(t0, t1);
        }
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min) {
          return false;
        }

        inv = 1.0 / d.z;
        t0  = (min.z - o.z) * inv;
        t1  = (max.z - o.z) * inv;
        if (inv < 0.0) {
          std::swap(t0, t1);
        }
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        return t_max > t_min;
      }
    };

    [[nodiscard]] AABB surrounding_box(AABB const & a, AABB const & b) noexcept {
      AABB result;
      result.min = {std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y),
                    std::min(a.min.z, b.min.z)};
      result.max = {std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y),
                    std::max(a.max.z, b.max.z)};
      return result;
    }

    struct SceneAcceleration {
      enum class Type { Sphere, Cylinder };

      struct ObjectRef {
        Type type;
        std::size_t index;
        AABB bounds;
        Vector3 centroid;
      };

      struct Node {
        AABB bounds;
        int left{-1};
        int right{-1};
        int object_index{-1};

        [[nodiscard]] bool is_leaf() const noexcept { return object_index >= 0; }
      };

      std::vector<ObjectRef> objects;
      std::vector<Node> nodes;
      int root{-1};
    };

    [[nodiscard]] std::size_t order_value(std::vector<int> const & order, int position) noexcept {
      auto const pos = static_cast<std::size_t>(position);
      return static_cast<std::size_t>(order[pos]);
    }

    [[nodiscard]] int create_leaf(SceneAcceleration & accel, std::size_t object_index) {
      auto const node_idx = static_cast<int>(accel.nodes.size());
      accel.nodes.push_back({});
      auto & node       = accel.nodes.back();
      node.bounds       = accel.objects[object_index].bounds;
      node.object_index = static_cast<int>(object_index);
      return node_idx;
    }

    struct CentroidRange {
      Vector3 min;
      Vector3 max;
    };

    [[nodiscard]] CentroidRange centroid_range(SceneAcceleration const & accel,
                                               std::vector<int> const & order, int start, int end) {
      auto const first_idx = order_value(order, start);
      Vector3 min          = accel.objects[first_idx].centroid;
      Vector3 max          = min;
      for (int i = start + 1; i < end; ++i) {
        auto const idx        = order_value(order, i);
        Vector3 const & value = accel.objects[idx].centroid;
        min.x                 = std::min(min.x, value.x);
        min.y                 = std::min(min.y, value.y);
        min.z                 = std::min(min.z, value.z);
        max.x                 = std::max(max.x, value.x);
        max.y                 = std::max(max.y, value.y);
        max.z                 = std::max(max.z, value.z);
      }
      return {min, max};
    }

    [[nodiscard]] AABB sphere_bounds(Sphere const & sphere) noexcept {
      Vector3 const & c = sphere.center();
      double const r    = sphere.radius();
      Vector3 const ext{r, r, r};
      return {c - ext, c + ext};
    }

    [[nodiscard]] AABB cylinder_bounds(Cylinder const & cyl) noexcept {
      Vector3 const & c    = cyl.center();
      Vector3 const & axis = cyl.axis();
      double const half_h  = cyl.half_height();
      double const r       = cyl.radius();

      auto const extent_axis = [&](double comp) {
        double const axial  = std::abs(comp) * half_h;
        double const radial = r * std::sqrt(std::max(0.0, 1.0 - comp * comp));
        return axial + radial;
      };

      double const ex = extent_axis(axis.x);
      double const ey = extent_axis(axis.y);
      double const ez = extent_axis(axis.z);

      return {
        {c.x - ex, c.y - ey, c.z - ez},
        {c.x + ex, c.y + ey, c.z + ez}
      };
    }

    [[nodiscard]] int axis_with_greatest_extent(Vector3 const & extent) noexcept {
      if (extent.x >= extent.y and extent.x >= extent.z) {
        return 0;
      }
      if (extent.y >= extent.z) {
        return 1;
      }
      return 2;
    }

    int build_bvh(SceneAcceleration & accel, std::vector<int> & order, int start, int end) {
      if (end - start == 1) {
        auto const order_idx = order_value(order, start);
        return create_leaf(accel, order_idx);
      }

      auto const [centroid_min, centroid_max] = centroid_range(accel, order, start, end);
      Vector3 const extent                    = centroid_max - centroid_min;
      int const axis                          = axis_with_greatest_extent(extent);
      auto const comparator                   = [&](int lhs, int rhs) {
        auto const lhs_idx = static_cast<std::size_t>(lhs);
        auto const rhs_idx = static_cast<std::size_t>(rhs);
        return accel.objects[lhs_idx].centroid[axis] < accel.objects[rhs_idx].centroid[axis];
      };
      auto const begin_part = order.begin() + start;
      auto const end_part   = order.begin() + end;
      std::ranges::sort(begin_part, end_part, comparator);

      int const mid   = start + (end - start) / 2;
      int const left  = build_bvh(accel, order, start, mid);
      int const right = build_bvh(accel, order, mid, end);

      auto const node_idx = static_cast<int>(accel.nodes.size());
      accel.nodes.push_back({});
      auto & node = accel.nodes.back();
      node.left   = left;
      node.right  = right;
      node.bounds = surrounding_box(accel.nodes[static_cast<std::size_t>(left)].bounds,
                                    accel.nodes[static_cast<std::size_t>(right)].bounds);
      return node_idx;
    }

    [[nodiscard]] std::unique_ptr<SceneAcceleration> build_scene_acceleration(Scene const & scene) {
      auto accel = std::make_unique<SceneAcceleration>();
      accel->objects.reserve(scene.spheres.size() + scene.cylinders.size());

      for (std::size_t i = 0; i < scene.spheres.size(); ++i) {
        auto const box         = sphere_bounds(scene.spheres[i]);
        Vector3 const centroid = (box.min + box.max) * 0.5;
        accel->objects.push_back({SceneAcceleration::Type::Sphere, i, box, centroid});
      }
      for (std::size_t i = 0; i < scene.cylinders.size(); ++i) {
        auto const box         = cylinder_bounds(scene.cylinders[i]);
        Vector3 const centroid = (box.min + box.max) * 0.5;
        accel->objects.push_back({SceneAcceleration::Type::Cylinder, i, box, centroid});
      }

      if (!accel->objects.empty()) {
        std::vector<int> order(accel->objects.size());
        std::ranges::iota(order, 0);
        accel->root = build_bvh(*accel, order, 0, static_cast<int>(order.size()));
      }

      return accel;
    }

    struct AccelCacheEntry {
      Scene const * scene{};
      std::uint64_t generation{};
      std::size_t sphere_count{};
      std::size_t cylinder_count{};
      std::unique_ptr<SceneAcceleration> accel;
    };

    SceneAcceleration * get_cached_acceleration(Scene const & scene) {
      static std::vector<AccelCacheEntry> cache;
      auto it = std::ranges::find_if(
          cache, [&](AccelCacheEntry const & entry) { return entry.scene == &scene; });
      if (it == cache.end()) {
        cache.push_back({});
        it        = cache.end() - 1;
        it->scene = &scene;
      }
      if (it->accel == nullptr or it->generation != scene.generation or
          it->sphere_count != scene.spheres.size() or
          it->cylinder_count != scene.cylinders.size())
      {
        it->accel          = build_scene_acceleration(scene);
        it->generation     = scene.generation;
        it->sphere_count   = scene.spheres.size();
        it->cylinder_count = scene.cylinders.size();
      }
      return it->accel.get();
    }

    [[nodiscard]] bool sphere_hit_detail(Sphere const & sphere, Ray const & ray, double max_t,
                                         HitRecord & rec) {
      Vector3 const & center = sphere.center();
      Vector3 const oc       = ray.origin() - center;  // Vector from center to ray origin
      Vector3 const & d      = ray.direction();
      double const radius_sq = sphere.radius_squared();

      double const half_b       = oc.dot(d);
      double const c            = oc.dot(oc) - radius_sq;
      double const discriminant = half_b * half_b - c;
      if (discriminant < 0.0) {
        return false;  // No real roots → no hit
      }

      double const sq = std::sqrt(discriminant);
      double t        = -half_b - sq;  // Near intersection
      if (t <= EPS) {
        t = -half_b + sq;  // Far intersection
      }

      // Select smallest positive t that is > EPS
      if (t <= EPS or t >= max_t) {
        return false;
      }

      rec.t                = t;
      Vector3 const offset = oc + d * t;
      rec.point            = center + offset;

      // Compute outward normal and orient it properly
      Vector3 const outward_normal = offset * sphere.inv_radius();
      rec.set_face_normal(d, outward_normal);

      rec.is_cap_hit = false;
      rec.mat        = &sphere.material();
      return true;
    }

  }  // namespace

  // SPHERE HIT: Computes the intersection between a ray and a sphere.
  std::optional<HitRecord> hit_sphere(Sphere const & sphere, Ray const & ray, double max_t) {
    HitRecord rec;
    if (!sphere_hit_detail(sphere, ray, max_t, rec)) {
      return std::nullopt;
    }
    return rec;
  }

  // CYLINDER HIT:Computes the intersection between a ray and a cylinder.
  namespace {

    constexpr double NO_HIT = std::numeric_limits<double>::infinity();

    struct CylinderRayCache {
      Vector3 oc;
      double oc_dot_a{};
      double d_dot_a{};
      Vector3 d_perp;
      Vector3 oc_perp;
    };

    [[nodiscard]] CylinderRayCache make_cache(Cylinder const & cyl, Ray const & ray) {
      Vector3 const oc      = ray.origin() - cyl.center();
      Vector3 const & axis  = cyl.axis();
      Vector3 const & dir   = ray.direction();
      double const oc_dot_a = oc.dot(axis);
      double const d_dot_a  = dir.dot(axis);
      Vector3 const d_perp  = dir - axis * d_dot_a;
      Vector3 const oc_perp = oc - axis * oc_dot_a;
      return {oc, oc_dot_a, d_dot_a, d_perp, oc_perp};
    }

    double hit_cylinder_side(Cylinder const & cyl, CylinderRayCache const & cache, double max_t) {
      double const a = cache.d_perp.dot(cache.d_perp);
      if (a <= std::numeric_limits<double>::epsilon()) {
        return NO_HIT;  // No real intersection
      }
      double const half_b = cache.d_perp.dot(cache.oc_perp);
      double const c      = cache.oc_perp.dot(cache.oc_perp) - cyl.radius_squared();
      double const disc   = half_b * half_b - a * c;
      if (disc < 0.0) {
        return NO_HIT;
      }
      double const sq  = std::sqrt(disc);
      double const inv = 1.0 / a;
      double const t1  = (-half_b - sq) * inv;
      double const t2  = (-half_b + sq) * inv;
      double const t   = (t1 > EPS) ? t1 : t2;
      if (t <= EPS or t >= max_t) {
        return NO_HIT;  // Behind camera or invalid
      }
      double const y = cache.oc_dot_a + t * cache.d_dot_a;  // Distance along axis in scalar form
      if (std::abs(y) <= cyl.half_height()) {
        return t;  // Valid side hit
      }
      return NO_HIT;  // Outside cylinder caps
    }

    struct CapData {
      Vector3 center;
      Vector3 normal;
      double radius_sq{};
    };

    // Intersects a horizontal disk (cap)
    double hit_cylinder_cap(CapData const & cap, Ray const & ray, double max_t) {
      Vector3 const & O = ray.origin();
      Vector3 const & D = ray.direction();

      double const denom = D.dot(cap.normal);
      if (std::abs(denom) <= 1e-8) {
        return NO_HIT;  // Parallel to cap
      }

      double const t = (cap.center - O).dot(cap.normal) / denom;
      if (t <= EPS or t >= max_t) {
        return NO_HIT;  // Behind camera or too close
      }
      // Check radial distance against cap radius squared (with small tolerance)
      double const radial_sq = (O + D * t - cap.center).length_squared();
      if (radial_sq <= cap.radius_sq + 1e-6) {
        return t;
      }
      return NO_HIT;
    }

    enum class HitPart { None, Side, Top, Bottom };

    struct HitChoice {
      double t;
      HitPart part;
    };

    [[nodiscard]] std::optional<HitChoice> select_hit(double side, double top, double bottom) {
      double best              = NO_HIT;
      HitPart part             = HitPart::None;
      auto const try_candidate = [&](double candidate, HitPart which) {
        if (candidate < best) {
          best = candidate;
          part = which;
        }
      };
      try_candidate(side, HitPart::Side);
      try_candidate(top, HitPart::Top);
      try_candidate(bottom, HitPart::Bottom);
      if (!std::isfinite(best)) {
        return std::nullopt;
      }
      return HitChoice{best, part};
    }

    [[nodiscard]] bool cylinder_hit_detail(Cylinder const & cyl, Ray const & ray, double max_t,
                                           HitRecord & rec) {
      Vector3 const & C            = cyl.center();
      Vector3 const & A            = cyl.axis();
      double const half_height     = cyl.half_height();
      Vector3 const top_center     = C + A * half_height;
      Vector3 const bot_center     = C - A * half_height;
      CylinderRayCache const cache = make_cache(cyl, ray);
      auto const t_side            = hit_cylinder_side(cyl, cache, max_t);
      CapData const top_cap{top_center, A, cyl.radius_squared()};
      CapData const bot_cap{bot_center, -A, cyl.radius_squared()};
      auto const t_top  = hit_cylinder_cap(top_cap, ray, max_t);
      auto const t_bot  = hit_cylinder_cap(bot_cap, ray, max_t);
      auto const choice = select_hit(t_side, t_top, t_bot);
      if (!choice or choice->t >= max_t) {
        return false;  // No intersection at all
      }
      rec.t          = choice->t;
      rec.point      = ray.at(choice->t);
      rec.is_cap_hit = (choice->part != HitPart::Side);
      Vector3 outward_normal{};  // Determine which part generated the hit and compute normal
      switch (choice->part) {
        case HitPart::Side:
        {
          Vector3 const diff   = rec.point - C;
          Vector3 const radial = diff - A * diff.dot(A);
          // Use non-normalized radial vector to avoid artifacts in refractive shading
          outward_normal = radial;
          break;
        }
        case HitPart::Top:    outward_normal = A; break;
        case HitPart::Bottom: outward_normal = -A; break;
        case HitPart::None:   break;
      }
      rec.set_face_normal(ray.direction(), outward_normal);
      rec.mat = &cyl.material();
      return true;
    }

  };  // namespace

  std::optional<HitRecord> hit_cylinder(Cylinder const & cyl, Ray const & ray, double max_t) {
    HitRecord rec;
    if (!cylinder_hit_detail(cyl, ray, max_t, rec)) {
      return std::nullopt;
    }
    return rec;
  }

  namespace {

    struct TraversalContext {
      SceneAcceleration const * accel{};
      Scene const * scene{};
    };

    struct LeafState {
      double * closest{};
      HitRecord * best_hit{};
    };

    [[nodiscard]] bool process_leaf(TraversalContext const & ctx,
                                    SceneAcceleration::Node const & node, Ray const & ray,
                                    LeafState state) {
      auto const & obj = ctx.accel->objects[static_cast<std::size_t>(node.object_index)];
      HitRecord candidate{};
      bool hit = false;
      if (obj.type == SceneAcceleration::Type::Sphere) {
        hit = sphere_hit_detail(ctx.scene->spheres[obj.index], ray, *state.closest, candidate);
      } else {
        hit = cylinder_hit_detail(ctx.scene->cylinders[obj.index], ray, *state.closest, candidate);
      }
      if (!hit) {
        return false;
      }
      *state.closest  = candidate.t;
      *state.best_hit = candidate;
      return true;
    }

    [[nodiscard]] bool traverse_bvh(TraversalContext const & ctx, Ray const & ray, double & closest,
                                    HitRecord & best_hit) {
      thread_local ::std::vector<int> stack;
      stack.clear();
      stack.reserve(ctx.accel->nodes.size());
      stack.push_back(ctx.accel->root);
      bool has_hit = false;
      LeafState const state{&closest, &best_hit};
      while (!stack.empty()) {
        int const node_idx = stack.back();
        stack.pop_back();
        SceneAcceleration::Node const & node = ctx.accel->nodes[static_cast<std::size_t>(node_idx)];
        if (!node.bounds.hit(ray, EPS, closest)) {
          continue;
        }
        if (node.is_leaf()) {
          has_hit = process_leaf(ctx, node, ray, state) or has_hit;
        } else {
          if (node.left >= 0) {
            stack.push_back(node.left);
          }
          if (node.right >= 0) {
            stack.push_back(node.right);
          }
        }
      }
      return has_hit;
    }

  }  // namespace

  // Traverses the acceleration structure to find the closest hit.
  bool first_hit(Scene const & scene, Ray const & ray, HitRecord & out_hit) {
    SceneAcceleration * accel = get_cached_acceleration(scene);
    if (accel == nullptr or accel->root < 0) {
      return false;
    }

    double closest = std::numeric_limits<double>::infinity();
    HitRecord best_hit{};
    TraversalContext const ctx{accel, &scene};
    bool const has_hit = traverse_bvh(ctx, ray, closest, best_hit);
    if (has_hit) {
      out_hit = best_hit;
    }
    return has_hit;
  }

  void prepare_scene_acceleration(Scene const & scene) {
    static_cast<void>(get_cached_acceleration(scene));
  }

};  // namespace render
