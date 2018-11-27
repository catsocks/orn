#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

int randint(int min, int max)
{
   return min + rand() % (max + 1 - min);
}

struct grid {
    int cell_size;
    int cell_outer_size;
    int border_width;
    int width;
    int height;
};

SDL_Rect cell_rect(struct grid *grid, float wd, float hd)
{
    return (SDL_Rect) {
        .x = grid->border_width + (int)(grid->width / wd) *
                grid->cell_outer_size,
        .y = grid->border_width + (int)(grid->height / hd) *
                grid->cell_outer_size,
        .w = grid->cell_size,
        .h = grid->cell_size
    };
}

SDL_Rect sprite_rect(int tile_size, int x, int y)
{
    return (SDL_Rect) {
        .x = tile_size * x,
        .y = tile_size * y,
        .w = tile_size,
        .h = tile_size
    };
}

int main()
{
    struct grid grid = {
        .cell_size = 36,
        .border_width = 1,
        .cell_outer_size = 0,
        .width = 27,
        .height = 21,
    };

    grid.cell_outer_size = grid.cell_size + grid.border_width;

    int window_width = (grid.width * grid.cell_outer_size) + grid.border_width;
    int window_height = (grid.height * grid.cell_outer_size) +
            grid.border_width;

    int grid_ghost_cursor_border_width = 3;

    SDL_Rect grid_ghost_cursor = cell_rect(&grid, 2, 2);

    SDL_Rect cell_question_mark = cell_rect(&grid, 1.5, 5);
    SDL_Rect cell_orn_1 = cell_rect(&grid, 2, 2);
    SDL_Rect cell_orn_2 = cell_rect(&grid, 4, 4);

    int sprites_tile_size = 36;

    SDL_Rect sprite_question_mark = sprite_rect(sprites_tile_size, 1, 0);
    SDL_Rect sprite_orn_1 = sprite_rect(sprites_tile_size, 0, 0);
    SDL_Rect sprite_orn_2 = sprite_rect(sprites_tile_size, 2, 0);

    unsigned cell_orn_2_sprite_timeout = 0;

    SDL_Color grid_background = {22, 22, 22, 255}; // Barely black
    SDL_Color grid_line_color = {44, 44, 44, 255}; // Dark grey
    SDL_Color grid_ghost_cursor_color = {44, 44, 44, 255};
    SDL_Color grid_ghost_cursor_border_color = {100, 100, 100, 255}; // Grey

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
                     SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL_image: %s",
                     IMG_GetError());
        return 1;
    }

    SDL_Surface *sprites = IMG_Load("assets/spritesheet.png");
    if (sprites == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Open spritesheet: %s",
                     IMG_GetError());
        return 1;
    }

    SDL_Cursor *cursor;
    cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    if (cursor == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Get the system hand cursor: %s", SDL_GetError());
    }
    SDL_SetCursor(cursor);

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (SDL_CreateWindowAndRenderer(window_width, window_height,
                                    SDL_WINDOW_RESIZABLE, &window,
                                    &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_SetWindowTitle(window, "Orn");

    SDL_Texture *sprites_texture = SDL_CreateTextureFromSurface(renderer,
                                                                sprites);
    if (sprites_texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create sprites texture: %s", SDL_GetError());
        return 1;
    }

    SDL_bool quit = SDL_FALSE;
    SDL_bool mouse_active = SDL_FALSE;
    SDL_bool mouse_hover = SDL_FALSE;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                case SDLK_UP:
                    cell_orn_1.y -= grid.cell_size;
                    break;
                case SDLK_s:
                case SDLK_DOWN:
                    cell_orn_1.y += grid.cell_size;
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    cell_orn_1.x -= grid.cell_size;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    cell_orn_1.x += grid.cell_size;
                    break;

                case SDLK_RETURN:
                    if (SDL_GetWindowFlags(window) &
                            SDL_WINDOW_FULLSCREEN_DESKTOP)
                        SDL_SetWindowFullscreen(window, 0);
                    else
                        SDL_SetWindowFullscreen(window,
                                                SDL_WINDOW_FULLSCREEN_DESKTOP);
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                cell_orn_1.x = (event.motion.x / grid.cell_outer_size) *
                        grid.cell_outer_size + 1;
                cell_orn_1.y = (event.motion.y / grid.cell_outer_size) *
                        grid.cell_outer_size + 1;
                break;
            case SDL_MOUSEMOTION:
                grid_ghost_cursor.x = (event.motion.x / grid.cell_outer_size) *
                        grid.cell_outer_size + 1;
                grid_ghost_cursor.y = (event.motion.y / grid.cell_outer_size) *
                        grid.cell_outer_size + 1;

                if (!mouse_active)
                    mouse_active = SDL_TRUE;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_ENTER &&
                        !mouse_hover)
                    mouse_hover = SDL_TRUE;
                else if (event.window.event == SDL_WINDOWEVENT_LEAVE &&
                         mouse_hover)
                    mouse_hover = SDL_FALSE;

                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    window_width = event.window.data1;
                    window_height = event.window.data2;

                    grid.width = window_width / grid.cell_size;
                    grid.height = window_height / grid.cell_size;

                    mouse_active = SDL_FALSE;
                }
                break;
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;
            }
        }

        // Draw grid background.
        SDL_SetRenderDrawColor(renderer, grid_background.r, grid_background.g,
                               grid_background.b, grid_background.a);
        SDL_RenderClear(renderer);

        // Draw grid cells with varying shades of grey.
        SDL_Rect cell = {0, 0, grid.cell_size, grid.cell_size};

        for (int x = 0; x < grid.width; x++) {
            for (int y = 0; y < grid.height; y++) {
                cell.x = (x * grid.cell_outer_size) + grid.border_width;
                cell.y = (y * grid.cell_outer_size) + grid.border_width;

                srand((unsigned) cell.y);

                unsigned char offset = rand() % 4;

                SDL_SetRenderDrawColor(renderer, grid_background.r + offset,
                                       grid_background.g + offset,
                                       grid_background.b + offset,
                                       grid_background.a);

                SDL_RenderFillRect(renderer, &cell);
            }
        }

        // Draw grid cell borders.
        SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g,
                               grid_line_color.b, grid_line_color.a);

        for (int x = 0; x < 1 + grid.width * grid.cell_size;
             x += grid.cell_outer_size) {
            SDL_RenderDrawLine(renderer, x, 0, x, window_height);
        }

        for (int y = 0; y < 1 + grid.height * grid.cell_size;
             y += grid.cell_outer_size) {
            SDL_RenderDrawLine(renderer, 0, y, window_width, y);
        }

        // Draw ghost cursor.
        if (mouse_active && mouse_hover) {
            SDL_SetRenderDrawColor(renderer, grid_ghost_cursor_color.r,
                                   grid_ghost_cursor_color.g,
                                   grid_ghost_cursor_color.b,
                                   grid_ghost_cursor_color.a);
            SDL_RenderFillRect(renderer, &grid_ghost_cursor);
        }

        // Draw first orn tile.
        SDL_RenderCopy(renderer, sprites_texture, &sprite_orn_1,
                       &cell_orn_1);

        // Update and draw second orn tile.
        if (cell_orn_2_sprite_timeout < SDL_GetTicks()) {
            if (cell_orn_2_sprite_timeout != 0) {
                if (sprite_orn_2.x == grid.cell_size * 2)
                    sprite_orn_2.x = grid.cell_size * 3;
                else
                    sprite_orn_2.x = grid.cell_size * 2;
            }

            cell_orn_2_sprite_timeout += SDL_GetTicks() +
                    (unsigned)randint(1000, 2000);
        }

        SDL_RenderCopy(renderer, sprites_texture, &sprite_orn_2,
                       &cell_orn_2);

        // Update and draw the question mark tile.
        SDL_RenderCopy(renderer, sprites_texture, &sprite_question_mark,
                       &cell_question_mark);

        // Draw ghost cursor border.
        if (mouse_active && mouse_hover) {
            SDL_SetRenderDrawColor(renderer, grid_ghost_cursor_border_color.r,
                                   grid_ghost_cursor_border_color.g,
                                   grid_ghost_cursor_border_color.b,
                                   grid_ghost_cursor_border_color.a);

            // Top border.
            SDL_Rect grid_cursor_border = {
                .x = grid_ghost_cursor.x - grid_ghost_cursor_border_width,
                .y = grid_ghost_cursor.y - grid_ghost_cursor_border_width,
                .w = grid_ghost_cursor.w + grid_ghost_cursor_border_width * 2,
                .h = grid_ghost_cursor_border_width,
            };

            SDL_RenderFillRect(renderer, &grid_cursor_border);

            // Bottom border.
            grid_cursor_border.y = grid_ghost_cursor.y + grid_ghost_cursor.h;

            SDL_RenderFillRect(renderer, &grid_cursor_border);

            // Left border.
            grid_cursor_border.x = grid_ghost_cursor.x -
                    grid_ghost_cursor_border_width;
            grid_cursor_border.y = grid_ghost_cursor.y;
            grid_cursor_border.h = grid_ghost_cursor.h;
            grid_cursor_border.w = grid_ghost_cursor_border_width;

            SDL_RenderFillRect(renderer, &grid_cursor_border);

            // Right border.
            grid_cursor_border.x = grid_ghost_cursor.x + grid_ghost_cursor.w;

            SDL_RenderFillRect(renderer, &grid_cursor_border);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(sprites);
    SDL_DestroyTexture(sprites_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
