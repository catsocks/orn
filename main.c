#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>

int randint(int min, int max) { return min + rand() % (max + 1 - min); }

struct grid {
    int cell_size;
    int cell_outer_size;
    int border_width;
    int width;
    int height;
};

struct grid_obj {
    SDL_Rect transform;
    SDL_Rect sprite;
};

struct grid_obj create_grid_obj(struct grid *grid, int x, int y) {
    struct grid_obj obj;
    obj.transform.x = grid->border_width + (x * grid->cell_outer_size);
    obj.transform.y = grid->border_width + (y * grid->cell_outer_size);
    obj.transform.w = grid->cell_size;
    obj.transform.h = grid->cell_size;
    return obj;
}

void grid_obj_sprite(struct grid_obj *obj, int tile_size, int x, int y) {
    obj->sprite.x = tile_size * x;
    obj->sprite.y = tile_size * y;
    obj->sprite.w = tile_size;
    obj->sprite.h = tile_size;
}

bool grid_obj_overlap(struct grid_obj *objs[], int objs_num,
                      struct grid_obj *obj) {
    for (int i = 0; i < objs_num; i++) {
        if (objs[i]->transform.x == obj->transform.x &&
            objs[i]->transform.y == obj->transform.y)
            return true;
    }

    return false;
}

bool grid_obj_overlap_rect(struct grid_obj *objs[], int objs_num,
                           SDL_Rect *rect) {
    for (int i = 0; i < objs_num; i++) {
        if (objs[i]->transform.x == rect->x && objs[i]->transform.y == rect->y)
            return true;
    }

    return false;
}

void grid_obj_move(struct grid *grid, struct grid_obj *obj, unsigned ticks,
                   int dx, int dy) {
    static unsigned timeout = 0;

    if (timeout < ticks) {
        obj->transform.x += dx * grid->cell_outer_size;
        obj->transform.y += dy * grid->cell_outer_size;

        timeout = ticks + 120;
    }
}

int main() {
    int spritesheet_tile_size = 36;

    struct grid grid = {
        .cell_size = 36,
        .border_width = 1,
        .cell_outer_size = 0,
        .width = 27,
        .height = 21,
    };

    grid.cell_outer_size = grid.cell_size + grid.border_width;

    int ghost_cursor_border_width = 3;

    SDL_Rect grid_ghost_cursor = {0, 0, grid.cell_size, grid.cell_size};

    struct grid_obj obj_question_mark = create_grid_obj(&grid, 18, 4);
    struct grid_obj obj_orn_1 = create_grid_obj(&grid, 13, 10);
    struct grid_obj obj_orn_2 = create_grid_obj(&grid, 6, 5);

    struct grid_obj *static_objs[] = {&obj_orn_2, &obj_question_mark};
    int static_objs_num = 2;

    grid_obj_sprite(&obj_question_mark, spritesheet_tile_size, 1, 0);
    grid_obj_sprite(&obj_orn_1, spritesheet_tile_size, 0, 0);
    grid_obj_sprite(&obj_orn_2, spritesheet_tile_size, 2, 0);

    unsigned obj_orn_2_sprite_timeout = 0;

    SDL_Color grid_background = {22, 22, 22, 255};
    SDL_Color grid_line_color = {44, 44, 44, 255};

    SDL_Color ghost_cursor_focus_color = {44, 44, 44, 255};
    SDL_Color ghost_cursor_active_color = {88, 88, 88, 255};
    SDL_Color *ghost_cursor_color = &ghost_cursor_focus_color;

    SDL_Color ghost_cursor_border_focus_color = {100, 100, 100, 255};
    SDL_Color ghost_cursor_border_disabled_color = {44, 44, 44, 255};
    SDL_Color *ghost_cursor_border_color = &ghost_cursor_border_focus_color;

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

    SDL_Surface *spritesheet_surface = IMG_Load("assets/spritesheet.png");
    if (spritesheet_surface == NULL) {
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

    int window_width = (grid.width * grid.cell_outer_size) + grid.border_width;
    int window_height =
        (grid.height * grid.cell_outer_size) + grid.border_width;

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

    SDL_Texture *spritesheet_texture =
        SDL_CreateTextureFromSurface(renderer, spritesheet_surface);
    if (spritesheet_texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create spritesheet_surface texture: %s", SDL_GetError());
        return 1;
    }

    bool quit = false;
    bool mouse_active = false;
    bool mouse_hover = false;

    while (!quit) {
        unsigned ticks = SDL_GetTicks();

        int move_x = 0;
        int move_y = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                case SDLK_UP:
                    move_y = -1;
                    break;
                case SDLK_s:
                case SDLK_DOWN:
                    move_y = 1;
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    move_x = -1;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    move_x = 1;
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
                ghost_cursor_color = &ghost_cursor_active_color;
                break;
            case SDL_MOUSEBUTTONUP:
                if (ghost_cursor_color == &ghost_cursor_active_color)
                    ghost_cursor_color = &ghost_cursor_focus_color;

                if (event.motion.x >
                    obj_orn_1.transform.x + obj_orn_1.transform.w)
                    move_x = 1;
                else if (event.motion.x < obj_orn_1.transform.x)
                    move_x = -1;

                if (event.motion.y >
                    obj_orn_1.transform.y + obj_orn_1.transform.h)
                    move_y = 1;
                else if (event.motion.y < obj_orn_1.transform.y)
                    move_y = -1;
                break;
            case SDL_MOUSEMOTION:
                grid_ghost_cursor.x = (event.motion.x / grid.cell_outer_size) *
                                          grid.cell_outer_size +
                                      1;
                grid_ghost_cursor.y = (event.motion.y / grid.cell_outer_size) *
                                          grid.cell_outer_size +
                                      1;

                if (!mouse_active)
                    mouse_active = true;

                if (grid_obj_overlap_rect(static_objs, static_objs_num,
                                          &grid_ghost_cursor)) {
                    ghost_cursor_border_color =
                        &ghost_cursor_border_disabled_color;
                } else {
                    if (ghost_cursor_border_color ==
                        &ghost_cursor_border_disabled_color)
                        ghost_cursor_border_color =
                            &ghost_cursor_border_focus_color;
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_ENTER && !mouse_hover)
                    mouse_hover = true;
                else if (event.window.event == SDL_WINDOWEVENT_LEAVE &&
                         mouse_hover)
                    mouse_hover = false;

                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    window_width = event.window.data1;
                    window_height = event.window.data2;

                    grid.width = window_width / grid.cell_size;
                    grid.height = window_height / grid.cell_size;

                    mouse_active = false;
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
            }
        }

        if (move_x != 0 || move_y != 0) {
            int old_x = obj_orn_1.transform.x;
            int old_y = obj_orn_1.transform.y;

            grid_obj_move(&grid, &obj_orn_1, ticks, move_x, move_y);
            if (grid_obj_overlap(static_objs, static_objs_num, &obj_orn_1)) {
                obj_orn_1.transform.x = old_x;
                obj_orn_1.transform.y = old_y;
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

                srand((unsigned)cell.y);

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
            SDL_SetRenderDrawColor(renderer, ghost_cursor_color->r,
                                   ghost_cursor_color->g, ghost_cursor_color->b,
                                   ghost_cursor_color->a);
            SDL_RenderFillRect(renderer, &grid_ghost_cursor);
        }

        // Draw first orn tile.
        SDL_RenderCopy(renderer, spritesheet_texture, &obj_orn_1.sprite,
                       &obj_orn_1.transform);

        // Update and draw second orn tile.
        if (obj_orn_2_sprite_timeout < ticks) {
            if (obj_orn_2_sprite_timeout != 0) {
                if (obj_orn_2.sprite.x == grid.cell_size * 2)
                    obj_orn_2.sprite.x = grid.cell_size * 3;
                else
                    obj_orn_2.sprite.x = grid.cell_size * 2;
            }

            obj_orn_2_sprite_timeout += ticks + (unsigned)randint(1000, 2000);
        }

        SDL_RenderCopy(renderer, spritesheet_texture, &obj_orn_2.sprite,
                       &obj_orn_2.transform);

        // Update and draw the question mark tile.
        SDL_RenderCopy(renderer, spritesheet_texture, &obj_question_mark.sprite,
                       &obj_question_mark.transform);

        // Draw ghost cursor border.
        if (mouse_active && mouse_hover) {
            SDL_SetRenderDrawColor(renderer, ghost_cursor_border_color->r,
                                   ghost_cursor_border_color->g,
                                   ghost_cursor_border_color->b,
                                   ghost_cursor_border_color->a);

            // Top border.
            SDL_Rect grid_cursor_border = {
                .x = grid_ghost_cursor.x - ghost_cursor_border_width,
                .y = grid_ghost_cursor.y - ghost_cursor_border_width,
                .w = grid_ghost_cursor.w + ghost_cursor_border_width * 2,
                .h = ghost_cursor_border_width,
            };

            SDL_RenderFillRect(renderer, &grid_cursor_border);

            // Bottom border.
            grid_cursor_border.y = grid_ghost_cursor.y + grid_ghost_cursor.h;

            SDL_RenderFillRect(renderer, &grid_cursor_border);

            // Left border.
            grid_cursor_border.x =
                grid_ghost_cursor.x - ghost_cursor_border_width;
            grid_cursor_border.y = grid_ghost_cursor.y;
            grid_cursor_border.h = grid_ghost_cursor.h;
            grid_cursor_border.w = ghost_cursor_border_width;

            SDL_RenderFillRect(renderer, &grid_cursor_border);

            // Right border.
            grid_cursor_border.x = grid_ghost_cursor.x + grid_ghost_cursor.w;

            SDL_RenderFillRect(renderer, &grid_cursor_border);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(spritesheet_surface);
    SDL_DestroyTexture(spritesheet_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
