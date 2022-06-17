// NOTE: implementation based on https://adrianb.io/2014/08/09/perlinnoise.html

 
function fade(t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

function grad(hash, x, y, z) {
    switch (hash & 0xf) {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xa: return  y - z;
        case 0xb: return -y - z;
        case 0xc: return  y + x;
        case 0xd: return -y + z;
        case 0xe: return  y - x;
        case 0xf: return -y - z;
        default:  return 0; // never happens
    }
}

function generate_perlin_permutations() {
    let permutations = new Array(512);
    for (let i = 0; i < 256; i++) {
        permutations[i] = i;
    }
    for (let i = 0; i < 256; i++) {
        let index = i + Math.floor(Math.random() * (256 - i));
        let temp = permutations[i];
        permutations[i] = permutations[index];
        permutations[index] = temp;
    }
    for (let i = 0; i < 256; i++) {
        permutations[256+i] = permutations[i];
    }
       
    return permutations;
}


const perlin_noise_permutations = generate_perlin_permutations();

function perlin_noise(x, y, z) {
    const p = perlin_noise_permutations;

    const xi = x & 0xff;
    const yi = y & 0xff;
    const zi = z & 0xff;
    x -= Math.floor(x);
    y -= Math.floor(y);
    z -= Math.floor(z);
    const u = fade(x);
    const v = fade(y);
    const w = fade(z);

    const aaa = p[p[p[xi    ] + yi    ] + zi    ];
    const aba = p[p[p[xi    ] + yi + 1] + zi    ];
    const aab = p[p[p[xi    ] + yi    ] + zi + 1];
    const abb = p[p[p[xi    ] + yi + 1] + zi + 1];
    const baa = p[p[p[xi + 1] + yi    ] + zi    ];
    const bba = p[p[p[xi + 1] + yi + 1] + zi    ];
    const bab = p[p[p[xi + 1] + yi    ] + zi + 1];
    const bbb = p[p[p[xi + 1] + yi + 1] + zi + 1];

    let x1, x2, y1, y2;
    x1 = lerp(u, grad(aaa, x, y,     z), grad(baa, x - 1, y,     z));
    x2 = lerp(u, grad(aba, x, y - 1, z), grad(bba, x - 1, y - 1, z));
    y1 = lerp(v, x1, x2);

    x1 = lerp(u, grad(aab, x, y,     z - 1), grad(bab, x - 1, y,     z - 1));
    x2 = lerp(u, grad(abb, x, y - 1, z - 1), grad(bbb, x - 1, y - 1, z - 1));
    y2 = lerp(v, x1, x2);

    return (lerp(w, y1, y2) + 1)/2;
}


function octave_perlin_noise(x, y, z, octaves, persistance) {
    let total = 0.0;
    let frequency = 1.0;
    let amplitude = 1.0;
    let max_value = 0.0;
    for (let i = 0; i < octaves; i++) {
        total += perlin_noise(x * frequency, y * frequency, z * frequency) * amplitude;
        max_value += amplitude;
        amplitude *= persistance;
        frequency *= 2;
    }

    return total/max_value;
}

function generate_terrain_height_map(width,
                                     height,
                                     detail_x=100,
                                     detail_y=100,
                                     octave=8,
                                     persistance=0.33,
                                     max_terrain_height=10.0,
                                     min_terrain_height=-2.0) {

    let terrain = {};
    let terrain_height  = max_terrain_height - min_terrain_height;
    terrain.scale_x = detail_x / width;
    terrain.scale_y = detail_y / height;
    terrain.width = detail_x;
    terrain.height = detail_x*detail_y;
    terrain.data = [];
    
    for (let i = 0; i < detail_x * detail_y; i++) {
        let x = (i / detail_x)/20.0;
        let y = (i % detail_x)/20.0;
        terrain.data[i] = octave_perlin_noise(x, y, 1.0, octave, persistance);
        terrain.data[i] = terrain.data[i] * terrain_height * min_terrain_height;
    }
    
    return terrain
}

function sample_point_at(map, x, y) {
    const x0 = Math.floor(x*map.scale_x);
    const y0 = Math.floor(y*map.scale_y);

    if (x0 < 0) x0 = 0; if (x0 > map.width - 1)  x0 = map.width  - 1;
    if (y0 < 0) y0 = 0; if (y0 > map.height - 1) y0 = map.height - 1;
    
    let x1 = x0 + 1;
    let y1 = y0 + 1;

    if (x1 > map.width - 1)  x1 = map.width  - 1;
    if (y1 > map.height - 1) y1 = map.height - 1;

    const h1 = map.data[x0+y0*map.width];
    const h2 = map.data[x1+y0*map.width];
    const h3 = map.data[x0+y1*map.width];
    const h4 = map.data[x1+y1*map.width];
    
    const u = x*map.scale_x - x0;
    const v = y*map.scale_y - y0;
    const hx0 = lerp(u, h1, h2);
    const hx1 = lerp(u, h3, h4);
    return lerp(v, hx0, hx1);
}
