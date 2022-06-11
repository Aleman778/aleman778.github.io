

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

let time = 0.0;

function render_3d_terrain(scene) {
    const view_scroll = document.scrollingElement.scrollTop / 500;
    const sin_time = Math.sin(time);

    gl.useProgram(scene.program);
    const vw = viewport.width / 1000.0;
    const vh = viewport.height / 1000.0;
    
    const proj_matrix = perspective_mat4(-100.0, 1000.0, -vw, vw, -vh, vh);
    gl.uniformMatrix4fv(scene.uniform_projection, false, proj_matrix);

    const look_at_dist = 1.0;
    const look_at_angle = Math.PI/2.0 + view_scroll;

    const camera_pos = [0.0, -look_at_dist*Math.sin(look_at_angle), look_at_dist*Math.cos(look_at_angle)];
    const camera_origin = [0.0, 0.0, 0.0];

    // console.log(view_scroll);
    const speed = 0.02;
    const end_time = 4.0;

    let offset = time*speed;
    if (time > end_time) {
        offset = end_time*speed;
    }
    
    const look_at = look_at_mat4(camera_pos, camera_origin, [0.0, 0.0, 0.05 + offset * (look_at_dist*Math.cos(look_at_angle) + 1.0)]);
    const view_translate = translation_mat4(0.0,sin_time *4.0, 0.0);
    const view_matrix = look_at;

    let mvp_matrix = proj_matrix;
    // mul_mat4(view, view_matrix);
    let mvp_2_matrix = mul_mat4(view_matrix, view_translate);

    if (time < 0.01) {
        console.log(mvp_matrix);
    }
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


    time += 0.01; // TODO: should use delta time of animation loop
}

let active_scene = null;

function canvas_3d_render_loop() {
    gl.clearColor(0.12, 0.13, 0.14, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    requestNextFrame(canvas_3d_render_loop);
    if (active_scene != null) {
        render_3d_terrain(active_scene);
    }
}


function canvas_3d_main() {
    create_3d_canvas();
    
    active_scene = generate_3d_terrain();
    canvas_3d_render_loop();
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

//
// 3D vector
//

function sub_vec3(a, b) {
    return [a[0] - b[0], a[1] - b[1], a[2] - b[2]];
}

function normalize_vec3(v) {
    const dist = Math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    return [dist*v[0], dist*v[1], dist*v[2]];
}

function cross_product_vec3(a, b) {
    return [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ];
}

//
// 4x4 matrix
//

function perspective_mat4(n, f, l, r, t, b) {
    return [
        2*n / (r - l), 0,                (r + l) / (r - l),  0,
        0,             2 * n / (t - b),  (t + b) / (t - b),  0,
        0,             0,               -(f + n) / (f - n), -(2 * f * n) / (f - n),
        0,             0,               -1,                  0];
}

function identity_mat4(x, y, z) {
    return [
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    ];
}

function translation_mat4(x, y, z) {
    return [
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1,
    ];
}

function look_at_mat4(from, to, pos = [0.0, 0.0, 0.0], tmp = [0.0, 0.0, 1.0]) {
    const tmp_norm = normalize_vec3(tmp);
    const f = normalize_vec3(sub_vec3(to, from));
    const r = cross_product_vec3(tmp_norm, f);


    // const f = [ 0, 1.0017007269890779, 0.0023774305467727 ]
    // const r = [ 1.0823774305467727, 0, 0 ]
    console.log(f, r);
    // f = vec3(0.0f, -1.0f, 0.0f);
    // r = vec3(0.0f, 0.0f, 1.0f);
    const u = cross_product_vec3(f, r);


    return [
        r[0], r[1], r[2], 0,
        u[0], u[1], u[2], 0,
        f[0], f[1], f[2], 0,
        pos[0], pos[1], pos[2], 1,
    ];
}

// function look_at_mat4(from, to, tmp = [0.0, 1.0, 0.0]) {
//     const tmp_norm = normalize_vec3(tmp);
//     const f = normalize_vec3(sub_vec3(to, from));
//     const r = cross_product_vec3(tmp_norm, f);
//     const u = cross_product_vec3(f, r);

//     return [
//         r[0], u[0], f[0], 0,
//         r[1], u[1], f[1], 0,
//         r[2], u[2], f[2], 0,
//         0,    0,    0,    1,
//     ];
// }

function mul_mat4(a, b) {
    let result = [];
    let a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3];
    let a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7];
    let a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11];
    let a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

    // b col 0
    let b0 = b[0], b1 = b[4], b2 = b[8], b3 = b[12];
    result[0]  = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
    result[4]  = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
    result[8]  = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
    result[12] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;
    
    // b col 1
    b0 = b[1]; b1 = b[5]; b2 = b[9]; b3 = b[13];
    result[1]  = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
    result[5]  = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
    result[9]  = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
    result[13] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

    // b col 2
    b0 = b[2]; b1 = b[6]; b2 = b[10]; b3 = b[14];
    result[2]  = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
    result[6]  = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
    result[10] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
    result[14] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;
    

    // b row 3
    b0 = b[3]; b1 = b[7]; b2 = b[11]; b3 = b[15];
    result[3]  = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
    result[7]  = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
    result[11] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
    result[15] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

    return result;
}

function create_notification(html, modifier="m-info") {
    const target = document.getElementById("main-notification-container");
    const notification = document.createElement("div");
    notification.className = "notification " + modifier;
    notification.innerHTML = html;
    target.prepend(notification);
    console.log("test");
}


function main() {
    create_notification("WARNING: This page is currently under construction and is not done yet!", "m-warning")
    canvas_3d_main();
}



document.addEventListener("DOMContentLoaded", function(){
   main();
});
