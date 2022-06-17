


let delta_time;
let elapsed_time;
let first_timestamp;
let prev_timestamp;
let curr_scroll_top;


function parallaxEffect(timestamp, terrainScene) {
    if (first_timestamp === undefined) {
        first_timestamp = timestamp;
        prev_timestamp = timestamp;
    }
    
    curr_scroll_top = document.scrollingElement.scrollTop;
    delta_time = timestamp - prev_timestamp;
    prev_timestamp = timestamp;

    elapsed_time = timestamp - first_timestamp;

    // Phase 1 - 3D Terrain
    if (curr_scroll_top < 4000) {
        const scroll_factor = curr_scroll_top / 4000;
        render_3d_terrain(terrainScene, scroll_factor);
    // } else if (curr_scroll_top < 4000) {
        // render_red_screen();
    } else if (curr_scroll_top < 6000) {
        render_green_screen();
    } else if (curr_scroll_top < 8000) {
        render_blue_screen();
    }
}
