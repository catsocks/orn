#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

int main()
{
    int grid_border_width = 1;
    int grid_cell_size = 36;
    int grid_cell_outer_size = 36 + grid_border_width;
    int grid_width = 27;
    int grid_height = 21;

    // + 1 so that the last grid lines fit in the screen.
    int window_width = (grid_width * grid_cell_outer_size) + 1;
    int window_height = (grid_height * grid_cell_outer_size) + 1;

    SDL_Rect grid_cell = {0, 0, grid_cell_outer_size, grid_cell_outer_size};

    // Place the grid cursor in the middle of the screen.
    SDL_Rect grid_cursor = {
        .x = ((grid_width - 1) / 2 * grid_cell_outer_size) + grid_border_width,
        .y = ((grid_height - 1) / 2 * grid_cell_outer_size) + grid_border_width,
        .w = grid_cell_size,
        .h = grid_cell_size,
    };

    SDL_Rect grid_cursor_sprite = {0, 0, grid_cell_size, grid_cell_size};

    // The cursor ghost is a cursor that always shows in the cell below the
    // mouse cursor.
    SDL_Rect grid_cursor_ghost = {grid_cursor.x, grid_cursor.y, grid_cell_size,
                                  grid_cell_size};

    SDL_Color grid_background = {22, 22, 22, 255}; // Barely black
    SDL_Color grid_line_color = {44, 44, 44, 255}; // Dark grey
    SDL_Color grid_cursor_ghost_color = {44, 44, 44, 255};

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
                    grid_cursor.y -= grid_cell_size;
                    break;
                case SDLK_s:
                case SDLK_DOWN:
                    grid_cursor.y += grid_cell_size;
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    grid_cursor.x -= grid_cell_size;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    grid_cursor.x += grid_cell_size;
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
                grid_cursor.x = (event.motion.x / grid_cell_outer_size) *
                        grid_cell_outer_size + 1;
                grid_cursor.y = (event.motion.y / grid_cell_outer_size) *
                        grid_cell_outer_size + 1;
                break;
            case SDL_MOUSEMOTION:
                grid_cursor_ghost.x = (event.motion.x / grid_cell_outer_size) *
                        grid_cell_outer_size + 1;
                grid_cursor_ghost.y = (event.motion.y / grid_cell_outer_size) *
                        grid_cell_outer_size + 1;

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

                    grid_width = window_width / grid_cell_size;
                    grid_height = window_height / grid_cell_size;

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
        for (int x = 0; x < grid_width; x++) {
            for (int y = 0; y < grid_height; y++) {
                grid_cell.x = x * grid_cell_size;
                grid_cell.y = y * grid_cell_size;

                srand((unsigned) grid_cell.y);

                unsigned char offset = rand() % 4;

                SDL_SetRenderDrawColor(renderer, grid_background.r + offset,
                                       grid_background.g + offset,
                                       grid_background.b + offset,
                                       grid_background.a);

                SDL_RenderFillRect(renderer, &grid_cell);
            }
        }

        // Draw grid cell borders.
        SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g,
                               grid_line_color.b, grid_line_color.a);

        for (int x = 0; x < 1 + grid_width * grid_cell_size;
             x += grid_cell_outer_size) {
            SDL_RenderDrawLine(renderer, x, 0, x, window_height);
        }

        for (int y = 0; y < 1 + grid_height * grid_cell_size;
             y += grid_cell_outer_size) {
            SDL_RenderDrawLine(renderer, 0, y, window_width, y);
        }

        // Draw grid ghost cursor.
        if (mouse_active && mouse_hover) {
            SDL_SetRenderDrawColor(renderer, grid_cursor_ghost_color.r,
                                   grid_cursor_ghost_color.g,
                                   grid_cursor_ghost_color.b,
                                   grid_cursor_ghost_color.a);
            SDL_RenderFillRect(renderer, &grid_cursor_ghost);
        }

        // Draw grid cursor.
        SDL_RenderCopy(renderer, sprites_texture, &grid_cursor_sprite,
                       &grid_cursor);

        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(sprites);
    SDL_DestroyTexture(sprites_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
