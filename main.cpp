#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <sys/time.h>

#define FULLSCREEN
#define SCREEN_W    (1024)
#define SCREEN_H    (768)
//#define SCREEN_W    (720)
//#define SCREEN_H    (540)
//#define ALTERNATE_GAMMA

#define BLANK_CHAR  0x20

#define CHECK_ERROR() { GLint err = glGetError(); if(err != GL_NO_ERROR) { printf("Error %d at %s:%d\n", err, __FILE__, __LINE__); exit(1); } }
#define CHECK_GL_CALL(expr) expr; CHECK_ERROR()

#define CHECK_SHADER_ERROR(handle) { GLint compileSuccess = 0; glGetShaderiv(handle, GL_COMPILE_STATUS, &compileSuccess); if(!compileSuccess) { GLchar messages[256]; glGetShaderInfoLog(handle, sizeof(messages), 0, &messages[0]); printf("Shader compile error at %s:%d: %s\n", __FILE__, __LINE__, messages); exit(1); } }
#define CHECK_LINK_ERROR(handle) { GLint compileSuccess = 0; glGetProgramiv(handle, GL_LINK_STATUS, &compileSuccess); if(!compileSuccess) { GLchar messages[256]; glGetProgramInfoLog(handle, sizeof(messages), 0, &messages[0]); printf("Shader link error at %s:%d: %s\n", __FILE__, __LINE__, messages); exit(1); } }

double getCurrentTime(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + ((double)t.tv_usec)*1e-6;
}

typedef struct
{
    unsigned char imageTypeCode;
    short int imageWidth;
    short int imageHeight;
    unsigned char bitCount;
    unsigned char *imageData;
} TGAFILE;

bool LoadTGAFile(const char *filename, TGAFILE *tgaFile)
{
    FILE *filePtr;
    unsigned char ucharBad;
    short int sintBad;
    long imageSize;
    int colorMode;
    unsigned char colorSwap;

    filePtr = fopen(filename, "rb");
    if (filePtr == NULL)
    {
        return false;
    }

    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

    fread(&tgaFile->imageTypeCode, sizeof(unsigned char), 1, filePtr);

    if (tgaFile->imageTypeCode != 2 && tgaFile->imageTypeCode != 3)
    {
        fclose(filePtr);
        return false;
    }

    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);

    fread(&tgaFile->imageWidth, sizeof(short int), 1, filePtr);
    fread(&tgaFile->imageHeight, sizeof(short int), 1, filePtr);

    fread(&tgaFile->bitCount, sizeof(unsigned char), 1, filePtr);

    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

    colorMode = tgaFile->bitCount / 8;
    imageSize = tgaFile->imageWidth * tgaFile->imageHeight * colorMode;

    tgaFile->imageData = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

    fread(tgaFile->imageData, sizeof(unsigned char), imageSize, filePtr);

    for (int imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode)
    {
        colorSwap = tgaFile->imageData[imageIdx];
        tgaFile->imageData[imageIdx] = tgaFile->imageData[imageIdx + 2];
        tgaFile->imageData[imageIdx + 2] = colorSwap;
    }

    fclose(filePtr);
    return true;
}


typedef struct
{
    TGAFILE tga;
    int nRows;
    int nCols;
} Font;

bool LoadFont(Font* font, const char* imgFile, int nRows, int nCols)
{
    if(!LoadTGAFile(imgFile, &font->tga))
    {
        printf("Failed to load texture file!\n");
        return false;
    }

    font->nRows = nRows;
    font->nCols = nCols;

    font->tga.imageWidth;

    return true;
}


typedef struct
{
    float x;
    float y;
    float tx;
    float ty;
} Vertex;

void SetupRawQuadPosition(Vertex* vertexBuf,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3,
    float x4, float y4)
{
    vertexBuf[0].x = x1;
    vertexBuf[0].y = y1;
    vertexBuf[1].x = x2;
    vertexBuf[1].y = y2;
    vertexBuf[2].x = x3;
    vertexBuf[2].y = y3;
    vertexBuf[3].x = x4;
    vertexBuf[3].y = y4;
}

void SetupRawQuadTexCoord(Vertex* vertexBuf,
    float tx1, float ty1,
    float tx2, float ty2,
    float tx3, float ty3,
    float tx4, float ty4)
{
    vertexBuf[0].tx = tx1;
    vertexBuf[0].ty = ty1;
    vertexBuf[1].tx = tx2;
    vertexBuf[1].ty = ty2;
    vertexBuf[2].tx = tx3;
    vertexBuf[2].ty = ty3;
    vertexBuf[3].tx = tx4;
    vertexBuf[3].ty = ty4;
}

void SetupRawQuad(Vertex* vertexBuf,
  float x1, float y1, float tx1, float ty1,
  float x2, float y2, float tx2, float ty2,
  float x3, float y3, float tx3, float ty3,
  float x4, float y4, float tx4, float ty4)
{
    SetupRawQuadPosition(vertexBuf, x1, y1, x2, y2, x3, y3, x4, y4);
    SetupRawQuadTexCoord(vertexBuf, tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4);
}

void SetupQuad(Vertex* vertexBuf, float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    const float ty2 = ty + th;
    const float tx2 = tx + tw;
    const float ty1 = ty;
    const float tx1 = tx;
    const float x1 = x;
    const float y1 = y;
    const float x2 = x + w;
    const float y2 = y + h;

    vertexBuf[0].x = x1;
    vertexBuf[0].y = y1;
    vertexBuf[0].tx = tx1;
    vertexBuf[0].ty = ty1;

    vertexBuf[1].x = x2;
    vertexBuf[1].y = y1;
    vertexBuf[1].tx = tx2;
    vertexBuf[1].ty = ty1;

    vertexBuf[2].x = x1;
    vertexBuf[2].y = y2;
    vertexBuf[2].tx = tx1;
    vertexBuf[2].ty = ty2;

    vertexBuf[3].x = x2;
    vertexBuf[3].y = y2;
    vertexBuf[3].tx = tx2;
    vertexBuf[3].ty = ty2;
}

void SetupGlyph(Vertex* vertexBuf, Font* font, int charIdx, float x, float y, float w, float h)
{
    const int col = charIdx % font->nCols;
    const int row = charIdx / font->nCols;
    const float tw = (1.0f / font->nCols);
    const float th = (1.0f / font->nRows);
    const float tx = col * tw;
    const float ty = row * th;
    const float pi = 0.00f;
    const float tix = 16.f/17;
    const float tiy = 24.f/25;
    SetupQuad(vertexBuf, x+pi, y+pi, w-pi*2, h-pi*2, tx, ty, tw*tix, th*tiy);
}

void SetupString(Vertex* vertexBuf, Font* font, int* charIndices, int nCols, int nRows, float x, float y, float w, float h)
{
    const int nChars = nCols * nRows;
    const float charW = w / nCols;
    const float charH = h / nRows;
    for(int n = 0; n < nChars; n++)
    {
        SetupGlyph(&vertexBuf[n*4], font, charIndices[n], x+charW*(n%nCols), y+charH*(n/nCols), charW, charH);
    }
}

double gTime = 0;

void warp(float& x, float& y)
{
    float ox = x;
    float oy = y;
    //float d = sqrtf(ox*ox+oy*oy);
    float d = (ox*ox*oy*oy);
    const float warpAmt = 0.04f;
    x = ox + ox * d * -warpAmt;
    y = oy + oy * d * -warpAmt;
}

struct screenGrid
{
    int width;
    int height;
    Vertex* origPoints;
    Vertex* dynamicPoints;
    Vertex* jitterVector;
};

void screenGrid_init(screenGrid* g, int width, int height)
{
    g->width = width;
    g->height = height;
    g->origPoints = (Vertex*)malloc(sizeof(Vertex) * width * height);
    g->dynamicPoints = (Vertex*)malloc(sizeof(Vertex) * width * height);
    g->jitterVector = (Vertex*)malloc(sizeof(Vertex) * width * height);

    const float sx = 2.f / (width-1);
    const float sy = 2.f / (height-1);

    const float stx = 1.f / (width-1);
    const float sty = 1.f / (height-1);

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            float fx = -1.f + (float)x * sx;
            float fy = -1.f + (float)y * sy;
            float tx = (float)x * stx;
            float ty = (float)y * sty;
            float jfx = fx + 0.00015f;
            float jfy = fy;

            //float d = sqrtf(ox*ox+oy*oy);
            warp(fx, fy);
            warp(jfx, jfy);

            Vertex& orig = g->origPoints[y*g->width+x];
            Vertex& dyn = g->dynamicPoints[y*g->width+x];
            Vertex& jit = g->jitterVector[y*g->width+x];

            orig.x = fx * 0.5f + 0.5f;
            orig.y = fy * 0.5f + 0.5f;
            orig.tx = tx;
            orig.ty = ty;

            jit.x = jfx - fx;
            jit.y = jfy - fy;

            dyn = orig;
        }
    }
}


void screenGrid_update(screenGrid* g, Vertex* vertexBuf, float ox, float oy, float w, float h, float tx, float ty, float tw, float th)
{
    for(int y = 0; y < g->height; y++)
    {
        for(int x = 0; x < g->width; x++)
        {
            const Vertex& orig = g->origPoints[y*g->width+x];
            const Vertex& jit = g->jitterVector[y*g->width+x];
            Vertex& dyn = g->dynamicPoints[y*g->width+x];

            float jitAmt = sinf(y*0.3f + gTime*15.f) * 0.5f + (rand()%100)*0.035f;

            dyn.x = ox + (orig.x + jit.x * jitAmt) * w;
            dyn.y = oy + (orig.y + jit.y * jitAmt) * h;
            dyn.tx = tx + orig.tx * tw;
            dyn.ty = ty + orig.ty * th;
        }
    }

    GLuint vidx = 0;
    for(int y = 0; y < g->height-1; y++)
    {
        for(int x = 0; x < g->width-1; x++)
        {
            vertexBuf[vidx++] = g->dynamicPoints[(y+0)*g->width+(x+0)];
            vertexBuf[vidx++] = g->dynamicPoints[(y+0)*g->width+(x+1)];
            vertexBuf[vidx++] = g->dynamicPoints[(y+1)*g->width+(x+0)];
            vertexBuf[vidx++] = g->dynamicPoints[(y+1)*g->width+(x+1)];
        }
    }
}

// void SetupGrid(Vertex* vertexBuf, int nCols, int nRows, float x, float y, float w, float h, float tx, float ty, float tw, float th)
// {
//     const float tcw = tw / nCols;
//     const float tch = th / nRows;
//     const float cw = w / nCols;
//     const float ch = h / nRows;
//     for(int row = 0; row < nRows; row++)
//     {
//         for(int col = 0; col < nCols; col++)
//         {
//             float x1 = (x + col * cw);
//             float y1 = (y + row * ch);
//             float x4 = (x1 + cw);
//             float y4 = (y1 + ch);
//             float x2 = x4;
//             float y2 = y1;
//             float x3 = x1;
//             float y3 = y4;
//             float tx1 = tx + col * tcw;
//             float ty1 = ty + row * tch;
//             float tx4 = tx1 + tcw;
//             float ty4 = ty1 + tch;
//             float tx2 = tx4;
//             float ty2 = ty1;
//             float tx3 = tx1;
//             float ty3 = ty4;
//             warp(x1, y1);
//             warp(x2, y2);
//             warp(x3, y3);
//             warp(x4, y4);
//             SetupRawQuad(&vertexBuf[(nCols*row+col)*4],
//                x1, y1, tx1, ty1,
//                x2, y2, tx2, ty2,
//                x3, y3, tx3, ty3,
//                x4, y4, tx4, ty4);
//         }
//     }

// }

void myGlfwErrorCB(int error, const char* description)
{
    fputs(description, stderr);
}

// This is the callback we'll be registering with GLFW for keyboard handling.
// The only thing we're doing here is setting up the window to close when we press ESC
void myGlfwKeyCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void compileShader(const char* vertexSrc, const char* fragmentSrc, GLuint* retProgram)
{
    CHECK_GL_CALL( *retProgram = glCreateProgram() );
    CHECK_GL_CALL( GLuint fs = glCreateShader(GL_FRAGMENT_SHADER) );
    CHECK_GL_CALL( glShaderSource(fs, 1, &fragmentSrc, NULL) );
    glCompileShader(fs);
    CHECK_SHADER_ERROR(fs);

    CHECK_GL_CALL( GLuint vs = glCreateShader(GL_VERTEX_SHADER) );
    CHECK_GL_CALL( glShaderSource(vs, 1, &vertexSrc, NULL) );
    glCompileShader(vs);
    CHECK_SHADER_ERROR(vs);

    CHECK_GL_CALL( glAttachShader(*retProgram, vs) );
    CHECK_GL_CALL( glAttachShader(*retProgram, fs) );

    glLinkProgram(*retProgram);
    CHECK_LINK_ERROR(*retProgram);
}

#define SHADER_LINE(x)  x "\n"

static const char* vs_source =
//SHADER_LINE( "#version 110 core" )
SHADER_LINE( "attribute vec2 Position;" )
SHADER_LINE( "attribute vec2 TexCoord;" )
SHADER_LINE( "void main(void) {" )
SHADER_LINE( "    gl_Position = vec4(Position.x,-Position.y,0,1);" )
SHADER_LINE( "    gl_TexCoord[0] = vec4(TexCoord.x,1.-TexCoord.y,0,0);" )
SHADER_LINE( "}" );

static const char* fs_source_ghost =
//SHADER_LINE( "#version 110 core" )
SHADER_LINE( "uniform sampler2D tex;" )
SHADER_LINE( "uniform vec4 color;" )
SHADER_LINE( "uniform float ghostDist;" )
SHADER_LINE( "uniform float ghostAmt;" )
SHADER_LINE( "void main(void) {" )
SHADER_LINE( "    vec4 s1 = texture2D(tex, gl_TexCoord[0].xy);" )
SHADER_LINE( "    vec4 s2 = texture2D(tex, gl_TexCoord[0].xy + vec2(-ghostDist,0.0));" )
SHADER_LINE( "    vec4 s3 = texture2D(tex, gl_TexCoord[0].xy + vec2(-ghostDist*2.,0.0));" )
SHADER_LINE( "    gl_FragColor = (s1 + (s2 + s3*0.3)*ghostAmt) * color;" )
//SHADER_LINE( "    gl_FragColor = vec4(1,1,1,1);" )
SHADER_LINE( "}" );

static const char * fs_source_normal =
//SHADER_LINE( "#version 110 core" )
SHADER_LINE( "uniform sampler2D tex;" )
SHADER_LINE( "uniform vec4 color;" )
SHADER_LINE( "uniform float ghostDist;" )
SHADER_LINE( "uniform float ghostAmt;" )
SHADER_LINE( "void main(void) {" )
SHADER_LINE( "    vec4 s1 = texture2D(tex, gl_TexCoord[0].xy);" )
SHADER_LINE( "    gl_FragColor = s1 * color;" )
//SHADER_LINE( "    gl_FragColor = vec4(1,1,1,1);" )
SHADER_LINE( "}" );


struct normalShader
{
    GLuint program;
    GLuint position;
    GLuint texCoord;
    GLuint texColor;
};

void normalShader_init(normalShader* s)
{
    compileShader(vs_source, fs_source_normal, &s->program);
    CHECK_GL_CALL( s->position = glGetAttribLocation(s->program, "Position") );
    CHECK_GL_CALL( s->texCoord = glGetAttribLocation(s->program, "TexCoord") );
    CHECK_GL_CALL( GLuint texUnit = glGetUniformLocation(s->program, "tex") );
    CHECK_GL_CALL( s->texColor = glGetUniformLocation(s->program, "color") );
    CHECK_GL_CALL( glUseProgram(s->program) );
    CHECK_GL_CALL( glUniform1i(texUnit, 0) );
}

void normalShader_select(normalShader* s)
{
    CHECK_GL_CALL( glUseProgram(s->program) );
    CHECK_GL_CALL( glEnableVertexAttribArray(s->position) );
    CHECK_GL_CALL( glEnableVertexAttribArray(s->texCoord) );
    CHECK_GL_CALL( glVertexAttribPointer(s->position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,x)) );
    CHECK_GL_CALL( glVertexAttribPointer(s->texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,tx)) );    
}


struct ghostShader
{
    GLuint program;
    GLuint position;
    GLuint texCoord;
    GLuint texColor;
    GLuint texGhostAmt;
    GLuint texGhostDist;
};

void ghostShader_init(ghostShader* s)
{
    compileShader(vs_source, fs_source_ghost, &s->program);
    CHECK_GL_CALL( s->position = glGetAttribLocation(s->program, "Position") );
    CHECK_GL_CALL( s->texCoord = glGetAttribLocation(s->program, "TexCoord") );
    CHECK_GL_CALL( GLuint texUnit = glGetUniformLocation(s->program, "tex") );
    CHECK_GL_CALL( s->texColor = glGetUniformLocation(s->program, "color") );
    CHECK_GL_CALL( s->texGhostDist = glGetUniformLocation(s->program, "ghostDist") );
    CHECK_GL_CALL( s->texGhostAmt = glGetUniformLocation(s->program, "ghostAmt") );
    CHECK_GL_CALL( glUseProgram(s->program) );
    CHECK_GL_CALL( glUniform1i(texUnit, 0) );
}

void ghostShader_select(ghostShader* s)
{
    CHECK_GL_CALL( glUseProgram(s->program) );
    CHECK_GL_CALL( glEnableVertexAttribArray(s->position) );
    CHECK_GL_CALL( glEnableVertexAttribArray(s->texCoord) );
    CHECK_GL_CALL( glVertexAttribPointer(s->position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,x)) );
    CHECK_GL_CALL( glVertexAttribPointer(s->texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,tx)) );    
}


struct renderTarget
{
    GLuint fbo;
    GLuint tex;
    GLuint width;
    GLuint height;
};

void renderTarget_initScreen(renderTarget* r, GLuint width, GLuint height)
{
    r->width = width;
    r->height = height;

    r->fbo = 0;
    r->tex = 0;

    CHECK_GL_CALL( glBindFramebuffer(GL_FRAMEBUFFER, 0) );
}

void renderTarget_initFBO(renderTarget* r, GLuint width, GLuint height)
{
    r->width = width;
    r->height = height;

    CHECK_GL_CALL( glGenFramebuffers(1, &r->fbo) );
    CHECK_GL_CALL( glBindFramebuffer(GL_FRAMEBUFFER, r->fbo) );

    CHECK_GL_CALL( glGenTextures(1, &r->tex) );
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, r->tex) );
    CHECK_GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0) );
    CHECK_GL_CALL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
    CHECK_GL_CALL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST) );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    CHECK_GL_CALL( glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, r->tex, 0) );
    GLenum drawBufs[1] = {GL_COLOR_ATTACHMENT0};
    CHECK_GL_CALL( glDrawBuffers(1, drawBufs) );
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed to setup render buffer\n");
        exit(1);
    }
}

void renderTarget_bind(renderTarget* r)
{
    CHECK_GL_CALL( glBindFramebuffer(GL_FRAMEBUFFER, r->fbo) );
    glViewport(0, 0, r->width, r->height);
}

void renderTarget_finalize(renderTarget* r)
{
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, r->tex) );
    glGenerateMipmap(GL_TEXTURE_2D);
}

void clearChars(int* charIndices, int rows, int cols, int ch)
{
    int total = rows * cols;
    while(total--)
    {
        *charIndices++ = ch;
    }
}

#define _CH(c,r) ((c)+(r)*16)

const int kLineChars[] = {
    _CH(0,4),
    _CH(2,4),
    _CH(3,4),
    _CH(4,4),
    _CH(5,4),
    _CH(6,4),
    _CH(7,4),
    _CH(8,4),
    _CH(12,4),
    _CH(15,4),
    _CH(0,5),
    _CH(2,5),
    _CH(4,5),
    _CH(9,5),
    _CH(11,5),
    _CH(13,5),
    _CH(3,6),
    _CH(4,6),
    _CH(5,6),
    _CH(7,6),
    _CH(11,6),
    _CH(13,6),
    _CH(14,6),
    _CH(0,7),
    _CH(1,7),
    _CH(2,7),
    _CH(3,7),
    _CH(10,7),
    _CH(13,7),
};

void updateChars(int* charIndices, int rows, int cols)
{
#define CHAR_AT(_r,_c) charIndices[(_r)*cols+(_c)]
    static int mode = 0;
    static int charSetLimit = 64;
    static int cursor = 0;
    static int tick = 0;
    static double lastUpdateTime = 0;

    const double now = getCurrentTime();

    if((now - lastUpdateTime) < (1./30))
    {
        return;
    }

    lastUpdateTime = now;
    ++tick;

#if 1
    if(rand()%100 == 0)
    {
        //clearChars(charIndices, rows, cols, BLANK_CHAR);
        int ct = 1 + rand()%(rows/2);
        for(int r = rows-ct; r < rows; r++)
        {
            for(int c = 0; c < cols; c++)
            {
                CHAR_AT(r,c) = BLANK_CHAR;
            }
        }
    }
    else if(rand()%1 == 0)
    {
        static int cur_r = 0;
        static int cur_c = 0;
        for(int n = 0; n < 10; n++)
        {
            cur_c = (cur_c+rand()%3-1)%cols;
            cur_r = (cur_r+rand()%3-1)%rows;
            //CHAR_AT(cur_r,cur_c) = (CHAR_AT(cur_r,cur_c)+rand()%5) % 0x100;
            CHAR_AT(cur_r,cur_c) = kLineChars[rand()%(sizeof(kLineChars)/sizeof(kLineChars[0]))];
        }
        for(int n = 0; n < 1000; n++)
        {
            int rc = rand() % cols;
            int rr = rand() % (rows-1);
            int above = CHAR_AT(rr,rc);
            int below = CHAR_AT(rr+1,rc);
            if(below == BLANK_CHAR)
            {
                CHAR_AT(rr+1,rc) = above;
                CHAR_AT(rr,rc) = below;
            }
        }
        // for(int r = 0; r < rows; r++)
        // {
        //     for(int c = 0; c < cols; c++)
        //     {
        //         //CHAR_AT(r,c) = kLineChars[rand()%(sizeof(kLineChars)/sizeof(kLineChars[0]))];
        //         //CHAR_AT(r,c) = (unsigned int)((sinf((tick+r)*0.1f)*cosf(tick+c)*0.5+0.5)*((c+1)*10))&0xFF;
        //     }
        // }
    }
    return;
#endif

    if(rand()%10 == 0) mode = rand()%6;

    if(rand()%20 == 0) charSetLimit = 32+(rand()%(256-32));

    
    if(mode == 0)
    {
        for(int cc = 0; cc < 1; cc++)
        {
            CHAR_AT(rows-1, cursor % cols) = rand() % charSetLimit;
            cursor += 1;
            if(cursor >= cols)
            {
                mode = 2;
                break;
            }
        }   
    }
    else if(mode == 1)
    {

        for(int nn = 0; nn < 10; nn++)
        {
            int dx = 0;
            int dy = 0;
            switch(rand() % 4)
            {
                case 0: dx = -1; break;
                case 1: dy = -1; break;
                case 2: dx = 1; break;
                case 3: dy = 1; break;
            }
            unsigned int x1 = rand() % cols;
            unsigned int y1 = rand() % rows;
            unsigned int x2 = (x1+dx) % cols;
            unsigned int y2 = (y1+dy) % rows;
            unsigned int tmp = CHAR_AT(y2, x2);
            CHAR_AT(y2, x2) = CHAR_AT(y1, x1);
            CHAR_AT(y1, x1) = tmp;
        }
    }
    else if(mode == 2)
    {
        mode = 0;
        for(int r = 0; r < rows-1; r++)
        {
            for(int c = 0; c < cols; c++)
            {
                CHAR_AT(r,c) = CHAR_AT(r+1,c);
            }
        }
        for(int c = 0; c < cols; c++)
        {
            CHAR_AT(rows-1,c) = BLANK_CHAR;
        }
        cursor = 0;
    }
}

int main()
{
  // Initialize GLFW, and if it fails to initialize for any reason, print it out to STDERR.
    if (!glfwInit())
    {
        fprintf(stderr, "Failed initialize GLFW.");
        exit(EXIT_FAILURE);
    }

    const int screenW = SCREEN_W;
    const int screenH = SCREEN_H;

    const int cols = 40;
    const int rows = 25;
    const int charPixW = 32;
    const int charPixH = 32;

    const int virtScreenW = cols * charPixW;
    const int virtScreenH = rows * charPixH;

    glfwSetErrorCallback(myGlfwErrorCB);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    //glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor* monitor = NULL;
#ifdef FULLSCREEN
    monitor = glfwGetPrimaryMonitor();
#endif
    GLFWwindow* window = glfwCreateWindow(screenW, screenH, "Phosphor", monitor, NULL);

    if(!window)
    {
        fprintf(stderr, "Failed to create GLFW window.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, myGlfwKeyCB);
    glfwSwapInterval(0);

    //printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

    glewExperimental = GL_TRUE;
    glewInit();




    int charIndices[rows][cols];

    for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            charIndices[r][c] = BLANK_CHAR;
        }
    }

    // const GLubyte Indices[] = {
    //     0, 1, 2,
    //     //2, 3, 0
    // };

    normalShader shNorm;
    normalShader_init(&shNorm);

    ghostShader shGhost;
    ghostShader_init(&shGhost);

    const int maxQuads = rows * cols;

    GLuint quadIndices[6*maxQuads];
    Vertex vertexBuf[4*maxQuads];

    for(int n = 0; n < maxQuads; n++)
    {
        quadIndices[n*6+0] = n*4 + 0;
        quadIndices[n*6+1] = n*4 + 1;
        quadIndices[n*6+2] = n*4 + 2;
        quadIndices[n*6+3] = n*4 + 1;
        quadIndices[n*6+4] = n*4 + 2;
        quadIndices[n*6+5] = n*4 + 3;
    }

    GLuint indexBuffer;
    CHECK_GL_CALL( glGenBuffers(1, &indexBuffer) );
    CHECK_GL_CALL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer) );
    CHECK_GL_CALL( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW) );


    GLuint vertexBuffer;
    CHECK_GL_CALL( glGenBuffers(1, &vertexBuffer) );
    CHECK_GL_CALL( glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer) );
    CHECK_GL_CALL( glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuf), vertexBuf, GL_DYNAMIC_DRAW) );

    Font font;
    if(!LoadFont(&font, "PetASCII4_mono.tga", 16, 16))
    {
        printf("Failed to load font file!\n");
        exit(1);
    }

    TGAFILE bgTga;
    if(!LoadTGAFile("MonitorBG.tga", &bgTga))
    {
        printf("Failed to load background!\n");
        exit(1);
    }

    TGAFILE scanTga;
    if(!LoadTGAFile("ScanBG.tga", &scanTga))
    {
        printf("Failed to load scan img!\n");
        exit(1);
    }

    CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );

    GLuint fontTex;
    CHECK_GL_CALL( glGenTextures(1, &fontTex) );
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, fontTex) );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    CHECK_GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, font.tga.imageWidth, font.tga.imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, font.tga.imageData) );
    //CHECK_GL_CALL( glGenerateMipmap(GL_TEXTURE_2D) );

    GLuint bgTexID;
    CHECK_GL_CALL( glGenTextures(1, &bgTexID) );
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, bgTexID) );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    CHECK_GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgTga.imageWidth, bgTga.imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgTga.imageData) );

    GLuint scanTexID;
    CHECK_GL_CALL( glGenTextures(1, &scanTexID) );
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, scanTexID) );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    CHECK_GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scanTga.imageWidth, scanTga.imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, scanTga.imageData) );

    renderTarget screen;
    renderTarget virtScreen;
    renderTarget phosphorLayer;
    renderTarget phosphorLayer2;
    renderTarget phosphorLayer3;

    renderTarget_initScreen(&screen, screenW, screenH);
    renderTarget_initFBO(&virtScreen, virtScreenW, virtScreenH);
    renderTarget_initFBO(&phosphorLayer, screenW, screenH);
    renderTarget_initFBO(&phosphorLayer2, screenW/4, screenH/4);
    renderTarget_initFBO(&phosphorLayer3, screenW/24, screenH/24);

    screenGrid grid;

    screenGrid_init(&grid, 20, 20);

    glEnable(GL_BLEND);

    int frameIdx = 0;

    normalShader_select(&shNorm);

    double lastFrameTime = getCurrentTime();
    
    while(!glfwWindowShouldClose(window))
    {
        const double now = getCurrentTime();
        const double frameDelta = (now - lastFrameTime);
        lastFrameTime = now;
        gTime += frameDelta;

        const float tcAdjust = 1.0f; //TODO: adjust time constants

        // for(int r = 0; r < rows; r++)
        // {
        //   for(int c = 0; c < cols; c++)
        //   {
        //     charIndices[r][c] = rand() % 256;
        //   }
        // }

        // for(int r = 0; r < 1; r++)
        // {
        //   charIndices[(frameIdx/cols)%rows][frameIdx%cols] = rand()%64;
        // }

        updateChars(&charIndices[0][0], rows, cols);

        /////////

        renderTarget_bind(&virtScreen);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SetupString(vertexBuf, &font, &charIndices[0][0], cols, rows, -1., -1., 2., 2.);

        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, 1.f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, fontTex) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6*rows*cols, GL_UNSIGNED_INT, 0) );

        const float scanAlpha = 0.08f;

        static float scanOfs = 0.0f;
        scanOfs += frameDelta * 0.5f * (1.f + (float)(rand()%30)/1000);
        while(scanOfs > 1.f) scanOfs -= 1.f;
        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, -scanOfs, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, scanAlpha) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, scanTexID) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );

        renderTarget_finalize(&virtScreen);

        /////////

        renderTarget_bind(&phosphorLayer);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        const float flickerA = 0.95f+0.05f*sinf(gTime*100.0f);

#if 1
        const float fontColorR = 0.3f;
        const float fontColorG = 1.0f;
        const float fontColorB = 0.3f;
#else
        const float fontColorR = 0.7f;
        const float fontColorG = 0.8f;
        const float fontColorB = 1.0f;
#endif

        ghostShader_select(&shGhost);

        static float driftGhost = 0.0f;
        driftGhost += (((float)(rand()%100)/100.f) - driftGhost) * 0.3f * tcAdjust;
        static float driftGhostAmt = 0.0f;
        driftGhostAmt += (((float)(rand()%100)/100.f) - driftGhostAmt) * 0.2f * tcAdjust;
        CHECK_GL_CALL( glUniform1f(shGhost.texGhostDist, 0.002f + 0.002f * driftGhost) );
        CHECK_GL_CALL( glUniform1f(shGhost.texGhostAmt, 0.15f * driftGhostAmt) );

        {
            const float scale = std::min( (float)screen.width/virtScreen.width, (float)screen.height/virtScreen.height) * 0.73f;
            const float sqX = (-scale*virtScreen.width)/screen.width;
            const float sqY = (-scale*virtScreen.height)/screen.height-0.09;
            const float sqW = (2*scale*virtScreen.width)/screen.width;
            const float sqH = (2*scale*virtScreen.height)/screen.height*1.2;
            //SetupGrid(vertexBuf, 20, 20, sqX, sqY, sqW, sqH, 0, 1, 1, -1);
            screenGrid_update(&grid, vertexBuf, sqX, sqY, sqW, sqH, 0, 0, 1, 1);
            CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
            CHECK_GL_CALL( glUniform4f(shGhost.texColor, fontColorR, fontColorG, fontColorB, 1.f) );
            CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
            CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
            CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, virtScreen.tex) );
            CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6*(grid.width-1)*(grid.height-1), GL_UNSIGNED_INT, 0) );
        }

        renderTarget_finalize(&phosphorLayer);

        /////////

        renderTarget_bind(&phosphorLayer2);

        normalShader_select(&shNorm);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, 1.0f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer.tex) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );

        renderTarget_finalize(&phosphorLayer2);

        /////////

        renderTarget_bind(&phosphorLayer3);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, 1.0f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer2.tex) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );

        renderTarget_finalize(&phosphorLayer3);

        /////////

        renderTarget_bind(&screen);

        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

#ifdef ALTERNATE_GAMMA
        float bgBrightness = 0.3f;
#else
        float bgBrightness = 1.115f;
#endif
        float brightness = 0.93f;
        float largeGlowAmt = 0.2f;
        float smallGlowAmt = 0.1f;
        float bgPersist = 0.2f;

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, bgBrightness, bgBrightness, bgBrightness, 1.f-bgPersist) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, bgTexID) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, brightness * 0.7f * flickerA) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer.tex) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, brightness * smallGlowAmt) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer2.tex) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        CHECK_GL_CALL( glUniform4f(shNorm.texColor, 1.f, 1.f, 1.f, brightness * largeGlowAmt) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer3.tex) );
        CHECK_GL_CALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );


        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frameIdx;
    }

    return 0;
}

// [END]
