APP_DIR  := churchfarnsworth
APP_LIBS := kernel network serverclient gfxclient ai/low kmlocal mapterrain pathfinding/simple_terrain pathfinding/dcdt/se pathfinding/dcdt/sr pathfinding/triangulation pathfinding/triangulation/gfxclient ai/movement

APP_EXT_HD   +=
APP_EXT_LIBS :=

APP := $(APP_DIR)
include config/app.rules
