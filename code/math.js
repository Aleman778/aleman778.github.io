
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
