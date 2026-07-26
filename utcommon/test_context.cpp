#include "../common/include/config.hpp"
#include "../common/include/context.hpp"
#include "../common/include/scene.hpp"
#include "../common/include/trace.hpp"
#include <gtest/gtest.h>

// Test: RenderContext must simply store pointers to Scene, Config, and Tracer
//       exactly as provided to its constructor.
TEST(test_context, stores_scene_config_and_tracer_pointers) {
  render::Scene const scene;    // Dummy scene instance
  render::Config const cfg;     // Dummy config instance
  render::Tracer tracer(1, 2);  // Tracer initialized with two RNG seeds

  // Construct a context pointing to scene, config, and tracer
  render::RenderContext const ctx{&scene, &cfg, &tracer};

  // Validate that stored pointers match the originals
  EXPECT_EQ(ctx.scene, &scene);
  EXPECT_EQ(ctx.cfg, &cfg);
  EXPECT_EQ(ctx.tracer, &tracer);
}
