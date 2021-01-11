#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tile_types.h"
#include "Tmx.h"

void write_word(short value, FILE *fp) {
  fputc((value & 0xff00) >> 8, fp);
  fputc(value & 0x00ff, fp);
}

char get_tile_type(const char *str)
{
  char result = TILE_TYPE_NONE;

  if (strcmp(str, "floor") == 0)
    result = TILE_TYPE_FLOOR;
  else if (strcmp(str, "rock") == 0)
    result = TILE_TYPE_ROCK;
  else if (strcmp(str, "metal") == 0)
    result = TILE_TYPE_METAL;
  else if (strcmp(str, "special_1") == 0)
    result = TILE_TYPE_SPECIAL_1;
  else if (strcmp(str, "special_2") == 0)
    result = TILE_TYPE_SPECIAL_2;
  else if (strcmp(str, "overlay") == 0)
    result = TILE_TYPE_OVERLAY;

  return result;
}

int get_max_tiles(const Tmx::Tileset *tileset)
{
  int w  = (tileset->GetImage())->GetWidth();
  int h  = (tileset->GetImage())->GetHeight();

  int tw = tileset->GetTileWidth();
  int th = tileset->GetTileHeight();

  int max_tiles_x = w / tw;
  int max_tiles_y = h / th;

  return max_tiles_x * max_tiles_y;
}


int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage is: %s <tmxfile> <binfile>\n", argv[0]);
    return 1;
  }

  printf("converting file: %s\n", argv[1]);
  Tmx::Map *map = new Tmx::Map();
printf("create map...\n");
  map->ParseFile(argv[1]);
printf("parsed map...\n");

  if (map->HasError()) {
    printf("error code: %d\n", map->GetErrorCode());
    printf("error text: %s\n", map->GetErrorText().c_str());
    return map->GetErrorCode();
  }

  printf("Version: %1.1f\n", map->GetVersion());

  if (map->GetOrientation() != Tmx::TMX_MO_ORTHOGONAL) {
    printf("error: map orientation must be orthogonal\n");
    return 1;
  }

  FILE *fp = fopen(argv[2], "wb");
  if (fp == NULL) {
    printf("error: unable to create file %s\n", argv[2]);
    return 1;
  }

  const Tmx::Tileset *tileset = map->GetTileset(0);
  if (!tileset) {
    printf("error - no tileset exist\n");
    return 1;
  }

  int num_tiles = get_max_tiles(tileset);
  printf("Number of tiles: %d\n", num_tiles);
  fputc((char) num_tiles, fp);

  std::vector<Tmx::Tile*> tiles = tileset->GetTiles();
  for (int i = 0; i < num_tiles; i++) {

    std::string value;
    char type = TILE_TYPE_NONE;

    for (int j = 0; j < tiles.size(); j++) {

      Tmx::Tile *tile = tiles[j];
      if (i == tile->GetId()) {

        const Tmx::PropertySet prop = tile->GetProperties();
        value = prop.GetLiteralProperty(std::string("type"));

        type = get_tile_type(value.c_str());
        if (type == TILE_TYPE_OVERLAY) {
          std::string value = prop.GetLiteralProperty(std::string("bg_tile"));
          char bg = (char) atoi(value.c_str());
          bg <<= 1;
          type |= bg;
        }

        printf("tile: %d %s(0x%x)\n", i, value.c_str(), type & 0xFF);
        break;
      }
    }

    fputc(type, fp);
  }

  const Tmx::Layer *layer = map->GetLayer(0);
  if (!layer) {
    printf("error: layer zero does not exist\n");
    return 1;
  }

  short w = (short) layer->GetWidth();
  short h = (short) layer->GetHeight();

  printf("Map size: %dx%d\n", w, h);

  write_word(w, fp);
  write_word(h, fp);

#ifdef VERTICAL
  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      char tile_id = (char) layer->GetTileId(x, y);
      fputc(tile_id, fp);
    }
  }
#else
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      char tile_id = (char) layer->GetTileId(x, y);
      fputc(tile_id, fp);
    }
  }
#endif

  return 0;
}

