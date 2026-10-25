### from just edges and vertices only
using non optimized compiler
render 6531 us, frame 41304 us, 24 FPS
using optimized compiler
render 1382 us, frame 36113 us, 27 FPS


### for torus ring, including mesh, rasterizer and the full renderer with z-frame buffer
-O2
render 11513 us, frame 46389 us, 21 FPS | tris/frame: 268 drawn, 307 culled, 0 skipped
-O0
render 41539 us, frame 76465 us, 13 FPS | tris/frame: 270 drawn, 305 culled, 0 skipped

with z buffer
-O2
render 21480 us, frame 56356 us, 17 FPS | tris/frame: 248 drawn, 327 culled, 0 skipped
-O0
render 56434 us, frame 91361 us, 10 FPS | tris/frame: 268 drawn, 307 culled, 0 skipped

with goraud
-O0
render 72850 us, frame 107777 us, 9 FPS | tris/frame: 268 drawn, 307 culled, 0 skipped
-O2
render 26607 us, frame 61482 us, 16 FPS | tris/frame: 268 drawn, 307 culled, 0 skipped
