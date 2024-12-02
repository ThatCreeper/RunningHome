#pragma once

#define TILE_SIZE (8)
#define LEVEL_WIDTH (TILE_SIZE*17)
#define LEVEL_HEIGHT (TILE_SIZE*9)

#define SCRWID 768
#define SCRHEI (432+32)

#include <raylib.h>
#include <rlgl.h>
#include <cmath>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "version.h"
#include "sound.h"
#include "gfx.h"
#include "helpers.h"
#include "globstate.h"