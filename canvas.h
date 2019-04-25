#pragma once

#include <emscripten.h>

EM_JS(void, canvas_setup, (int nx, int ny), {
    var ctx = window.ctx;
    ctx.canvas.width = nx;
    ctx.canvas.height = ny;
    var imageData = ctx.createImageData(nx, ny);
    var imageCanvas = document.createElement('canvas');
    imageCanvas.width = nx;
    imageCanvas.height = ny;
    window.imageData = imageData;
    window.imageCanvas = imageCanvas;
});

EM_JS(void, canvas_draw_data, (int* image, int nx, int ny, int ypos, int count), {
    var ctx = window.ctx;
    var imageData = window.imageData;
    var imageCanvas = window.imageCanvas;
    for (var y = ypos; y > Math.max(ypos-count, 0); y--) {
        for (var x = 0; x < nx; x++) {
            var i = 4 * (((ny - 1) - y) * nx + x); // 4 =  pitch
            var j = 3 * 4 * (y * nx + x); // 4 = sizeof(int), 3 = pitch
            imageData.data[i + 0] = HEAP32[(image + j + 0) >> 2];
            imageData.data[i + 1] = HEAP32[(image + j + 4) >> 2];
            imageData.data[i + 2] = HEAP32[(image + j + 8) >> 2];
            imageData.data[i + 3] = 255;
        }
    }
    ctx.putImageData(window.imageData, 0, 0);
    var imageCanvas = window.imageCanvas;
    imageCanvas.getContext("2d").putImageData(imageData, 0, 0);
});
