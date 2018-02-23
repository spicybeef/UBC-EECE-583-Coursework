#pragma once

// Program defaults
// Graphics constants
#define WIN_VIEWPORT_WIDTH                      1000.f
#define WIN_VIEWPORT_HEIGHT                     800.f
#define WIN_INFOPORT_WIDTH                      WIN_VIEWPORT_WIDTH
#define WIN_INFOPORT_HEIGHT                     100.f
#define WIN_GRAPHICPORT_WIDTH                   WIN_VIEWPORT_WIDTH
#define WIN_GRAPHICPORT_HEIGHT                  (WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT)
#define WIN_INFOPORT_PADDING                    10.f
// Grid constants
#define GRID_SHRINK_FACTOR                      0.8f
#define CELL_SHRINK_FACTOR                      0.7f