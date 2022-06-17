

let gl = null;
let viewport = { x: 0, y: 0, width: 0, height: 0 };

function resize_viewport_to_fit_canvas(canvas) {
    const displayWidth  = canvas.clientWidth;
    const displayHeight = canvas.clientHeight;

    const needResize = (canvas.width  !== displayWidth ||
                        canvas.height !== displayHeight);

    if (needResize) {
        canvas.width  = displayWidth;
        canvas.height = displayHeight;

        viewport.width = gl.drawingBufferWidth;
        viewport.height = gl.drawingBufferHeight;
        gl.viewport(viewport.x, viewport.y, viewport.width, viewport.height);
    }

    return needResize;
}
    
function create_3d_canvas() {
    const canvas = document.createElement("canvas");
    canvas.className = "main-canvas";
    document.body.append(canvas);

    window.addEventListener("resize", function(){
        // resize_viewport_to_fit_canvas(canvas);
        // TODO: need to re render the screen
    });

    gl = (canvas.getContext("webgl") ||
          canvas.getContext("experimental-webgl"));

    if (gl && gl instanceof WebGLRenderingContext) {
        create_notification("Congratulations! Your browser supports WebGL.");
        document.documentElement.style.backgroundColor = "transparent";
    } else {
        create_notification("Your browser or device may not support WebGL.", "m-error");
        return;
    }

    resize_viewport_to_fit_canvas(canvas);

    generate_3d_terrain();
}

function compile_shaders(vertex_source, fragment_source) {
    let vertex_shader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertex_shader, vertex_source);
    gl.compileShader(vertex_shader);
    
    let fragment_shader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragment_shader, fragment_source);
    gl.compileShader(fragment_shader);

    let program = gl.createProgram();
    gl.attachShader(program, vertex_shader);
    gl.attachShader(program, fragment_shader);
    gl.linkProgram(program);
    gl.detachShader(program, vertex_shader);
    gl.detachShader(program, fragment_shader);
    gl.deleteShader(vertex_shader);
    gl.deleteShader(fragment_shader);
    
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        var link_error = gl.getProgramInfoLog(program);
        create_notification("Shader program did not link successfully.", "m-error");
        console.error(link_error);
        return;
    }

    return program;
}


function generate_3d_terrain() {

    const width = 32*2;
    const height = 32*2;
    const vertex_size = 3; // x, y, z
    var vertices = new Float32Array(width*height*vertex_size);
    var indices = new Uint16Array(width*height*6);
    var indices_lines = new Uint16Array(width*height*6);

    let ii = 0;
    for (let i = 0; i < width*height; i++) {
        const x = i % width;
        const y = Math.floor(i / width);

        const vi = i*vertex_size;

        vertices[vi + 0] = (x / (width - 1)) * 2 - 1.0;
        vertices[vi + 1] = Math.random() * 0.05;
        vertices[vi + 2] = (y / (height - 1)) * 2 - 1.0;//0.0;

        if (x < width - 1 && y < height - 1) {
            // Line indices
            indices_lines[ii + 0] = i;
            indices_lines[ii + 1] = i + 1;

            indices_lines[ii + 2] = i;
            indices_lines[ii + 3] = i + width;

            indices_lines[ii + 4] = i + width;
            indices_lines[ii + 5] = i + 1;

            // Triangle indices
            indices[ii + 0] = i;
            indices[ii + 1] = i + 1;
            indices[ii + 2] = i + width;

            indices[ii + 3] = i + width;
            indices[ii + 4] = i + 1;
            indices[ii + 5] = i + width + 1;
            
            ii += 6;
        }
    }

    const vbo = gl.createBuffer();
    if (!vbo) {
        create_notification("Failed to create vertex buffer.", "m-error");
        return;
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    const ibo_lines = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo_lines);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices_lines, gl.STATIC_DRAW);

    const ibo = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

    const vertex_source = `
#version 100

precision highp float;

attribute vec3 position;

uniform mat4 mvp_matrix;
uniform mat4 mvp_matrix_2;

void main() {
    gl_Position = mvp_matrix * mvp_matrix_2 * vec4(position, 1.0);
    gl_PointSize = 6.0;
}
`;

    const fragment_source = `
#version 100
void main() {
  gl_FragColor = vec4(0.18, 0.54, 0.34, 1.0);
}
`;

    const program = compile_shaders(vertex_source, fragment_source);
    gl.useProgram(program);
    const uniform_mvp_matrix = gl.getUniformLocation(program, 'mvp_matrix');
    const uniform_mvp_matrix_2 = gl.getUniformLocation(program, 'mvp_matrix_2');

    return {
        vbo, ibo_lines, ibo, program, uniform_mvp_matrix, uniform_mvp_matrix_2,
        vertex_count: vertices.length / 3,
        index_count: indices.length,
    };
}

function render_3d_terrain(scene, view_scroll) {
    gl.clearColor(0.12, 0.13, 0.14, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    const sin_time = Math.sin(elapsed_time);

    gl.useProgram(scene.program);
    const vw = viewport.width / 1000.0;
    const vh = viewport.height / 1000.0;
    
    const proj_matrix = perspective_mat4(-100.0, 1000.0, -vw, vw, -vh, vh);
    gl.uniformMatrix4fv(scene.uniform_projection, false, proj_matrix);

    const look_at_dist = 1.0;
    const look_at_angle = Math.PI/2.0 + view_scroll;

    // Intro animation 4s
    const animation_time = 4000; // ms
    const distance = 0.3;
    const progress = elapsed_time / animation_time;
    let offset = distance - easeInOutCubic(progress)*distance;
    if (progress > 1.0) {
        offset = 0;
    }

    const camera_height = look_at_dist*Math.cos(look_at_angle);
    const camera_pos = [0.0, 0.0, 0.1 + camera_height];
    const camera_origin = [0.0, look_at_dist*Math.sin(look_at_angle), 0.0];

    const look_at = look_at_mat4(camera_pos, camera_origin, [
        0.0, 0.0, 0.1 + offset]);
    const view_matrix = look_at;

    let mvp_matrix = proj_matrix;
    let mvp_2_matrix = view_matrix;

    gl.uniformMatrix4fv(scene.uniform_mvp_matrix, false, mvp_matrix);
    gl.uniformMatrix4fv(scene.uniform_mvp_matrix_2, false, view_matrix);

    gl.bindBuffer(gl.ARRAY_BUFFER, scene.vbo);
    
    const position_attrib_index = 0;
    gl.enableVertexAttribArray(position_attrib_index);
    gl.vertexAttribPointer(position_attrib_index, 3, gl.FLOAT, false, 0, 0);


    // Draw depth buffer pass
    gl.enable(gl.DEPTH_TEST);
    gl.colorMask(false, false, false, false);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, scene.ibo);
    gl.drawElements(gl.TRIANGLES, scene.index_count, gl.UNSIGNED_SHORT, 0);
    gl.colorMask(true, true, true, true);

    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.FRONT_AND_BACK);
    gl.depthFunc(gl.LEQUAL);

    // Draw points
    gl.bindBuffer(gl.ARRAY_BUFFER, scene.vbo);
    gl.drawArrays(gl.POINTS, 0, scene.vertex_count);

    // Draw lines
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, scene.ibo_lines);
    gl.drawElements(gl.LINES, scene.index_count, gl.UNSIGNED_SHORT, 0);
    
    gl.disableVertexAttribArray(position_attrib_index);
    gl.disable(gl.CULL_FACE);
    gl.disable(gl.DEPTH_TEST);
}

function render_red_screen() {
    gl.clearColor(1.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
}

function render_green_screen() {
    gl.clearColor(0.0, 1.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
}

function render_blue_screen() {
    gl.clearColor(0.0, 0.0, 1.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
}

let active_scene = null;

function canvas_3d_render_loop(timestamp) {
    gl.clearColor(0.12, 0.13, 0.14, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    requestNextFrame(canvas_3d_render_loop);
    if (active_scene != null) {
        parallaxEffect(timestamp, active_scene);
    }
}


function canvas_3d_main() {
    create_3d_canvas();
    
    active_scene = generate_3d_terrain();
    requestNextFrame(canvas_3d_render_loop);
}

const requestNextFrame = (function() {
    return window.requestAnimationFrame || 
        window.webkitRequestAnimationFrame ||  
        window.mozRequestAnimationFrame || 
        window.oRequestAnimationFrame || 
        window.msRequestAnimationFrame ||
        
    // if none of the above, use non-native timeout method
    function(callback) {
        window.setTimeout(callback, 1000 / 60);
    };
})(); 
