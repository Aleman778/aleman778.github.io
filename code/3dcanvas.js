

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

    gl.clearColor(0.12, 0.13, 0.14, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);


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

function projection_matrix(n, f, l, r, t, b) {
    return [
        2*n / (r - l), 0,                (r + l) / (r - l),  0,
        0,             2 * n / (t - b),  (t + b) / (t - b),  0,
        0,             0,               -(f + n) / (f - n), -(2 * f * n) / (f - n),
        0,             0,               -1,                  0];
}

function generate_3d_terrain() {

    const width = 32;
    const height = 32;
    const vertex_size = 3; // x, y, z
    var vertices = new Float32Array(width*height*vertex_size);
    var indices = new Uint8Array(width*height*6);

    for (let i = 0; i < width*height; i++) {
        const x = i % width;
        const y = Math.floor(i / width);

        const vi = i*vertex_size;
        const ii = i*6;
        
        vertices[vi + 0] = (x / (width - 1));
        vertices[vi + 1] = (y / (height - 1));
        vertices[vi + 2] = 0.0;

        if (x < width - 15 && y < height - 25) {
            indices[ii + 0] = i;
            indices[ii + 1] = i + 1;

            indices[ii + 2] = i;
            indices[ii + 3] = i + width;

            indices[ii + 4] = i + width;
            indices[ii + 5] = i + 1;
        }
    }

    const vbo = gl.createBuffer();
    if (!vbo) {
        create_notification("Failed to create vertex buffer.", "m-error");
        return;
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    const ibo = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

    const vertex_source = `
#version 100

precision highp float;

attribute vec3 position;

uniform mat4 projection;

void main() {
    gl_Position = vec4(position, 1.0);
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

    // projection uniform
    const uniform_projection = gl.getUniformLocation(program, 'projection');
    const proj_matrix = projection_matrix(1.0, 100.0, -1.0, 1.0, -1.0, 1.0);
    gl.uniformMatrix4fv(uniform_projection, false, proj_matrix);

    const position_attrib_index = 0;
    gl.enableVertexAttribArray(position_attrib_index);
    gl.vertexAttribPointer(position_attrib_index, 3, gl.FLOAT, false, 0, 0);
    gl.drawElements(gl.LINES, indices.length, gl.UNSIGNED_BYTE, 0);
    gl.disableVertexAttribArray(position_attrib_index);

}

