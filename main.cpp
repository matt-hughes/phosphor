#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <cmath>

//#define FULLSCREEN

GLuint program;
GLuint vao;

#define CHECK_ERROR() { GLint err = glGetError(); if(err != GL_NO_ERROR) { printf("Error %d at %s:%d\n", err, __FILE__, __LINE__); exit(1); } }
#define CHECK_GL_CALL(expr) expr; CHECK_ERROR()

#define CHECK_SHADER_ERROR(handle) { GLint compileSuccess = 0; glGetShaderiv(handle, GL_COMPILE_STATUS, &compileSuccess); if(!compileSuccess) { GLchar messages[256]; glGetShaderInfoLog(handle, sizeof(messages), 0, &messages[0]); printf("Shader compile error at %s:%d: %s\n", __FILE__, __LINE__, messages); exit(1); } }
#define CHECK_LINK_ERROR(handle) { GLint compileSuccess = 0; glGetProgramiv(handle, GL_LINK_STATUS, &compileSuccess); if(!compileSuccess) { GLchar messages[256]; glGetProgramInfoLog(handle, sizeof(messages), 0, &messages[0]); printf("Shader link error at %s:%d: %s\n", __FILE__, __LINE__, messages); exit(1); } }

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
    float Position[2];
    float TexCoord[2];
} Vertex;


void SetupRawQuad(Vertex* vertexBuf,
  float x1, float y1, float tx1, float ty1,
  float x2, float y2, float tx2, float ty2,
  float x3, float y3, float tx3, float ty3,
  float x4, float y4, float tx4, float ty4)
{
    y1 = -y1;
    y2 = -y2;
    y3 = -y3;
    y4 = -y4;
    ty1 = 1.f - ty1;
    ty2 = 1.f - ty2;
    ty3 = 1.f - ty3;
    ty4 = 1.f - ty4;

    vertexBuf[0].Position[0] = x1;
    vertexBuf[0].Position[1] = y1;
    vertexBuf[0].TexCoord[0] = tx1;
    vertexBuf[0].TexCoord[1] = ty1;

    vertexBuf[1].Position[0] = x2;
    vertexBuf[1].Position[1] = y2;
    vertexBuf[1].TexCoord[0] = tx2;
    vertexBuf[1].TexCoord[1] = ty2;

    vertexBuf[2].Position[0] = x3;
    vertexBuf[2].Position[1] = y3;
    vertexBuf[2].TexCoord[0] = tx3;
    vertexBuf[2].TexCoord[1] = ty3;

    vertexBuf[3].Position[0] = x2;
    vertexBuf[3].Position[1] = y2;
    vertexBuf[3].TexCoord[0] = tx2;
    vertexBuf[3].TexCoord[1] = ty2;

    vertexBuf[4].Position[0] = x3;
    vertexBuf[4].Position[1] = y3;
    vertexBuf[4].TexCoord[0] = tx3;
    vertexBuf[4].TexCoord[1] = ty3;

    vertexBuf[5].Position[0] = x4;
    vertexBuf[5].Position[1] = y4;
    vertexBuf[5].TexCoord[0] = tx4;
    vertexBuf[5].TexCoord[1] = ty4;
}

void SetupQuad(Vertex* vertexBuf, float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    const float ty2 = 1.0f - ty;
    const float tx2 = tx + tw;
    const float ty1 = ty2 - th;
    const float y1 = -y;
    const float x2 = x + w;
    const float y2 = -(y + h);

    vertexBuf[0].Position[0] = x2;
    vertexBuf[0].Position[1] = y1;
    vertexBuf[0].TexCoord[0] = tx2;
    vertexBuf[0].TexCoord[1] = ty2;

    vertexBuf[1].Position[0] = x2;
    vertexBuf[1].Position[1] = y2;
    vertexBuf[1].TexCoord[0] = tx2;
    vertexBuf[1].TexCoord[1] = ty1;

    vertexBuf[2].Position[0] = x;
    vertexBuf[2].Position[1] = y2;
    vertexBuf[2].TexCoord[0] = tx;
    vertexBuf[2].TexCoord[1] = ty1;

    vertexBuf[3].Position[0] = x2;
    vertexBuf[3].Position[1] = y1;
    vertexBuf[3].TexCoord[0] = tx2;
    vertexBuf[3].TexCoord[1] = ty2;

    vertexBuf[4].Position[0] = x;
    vertexBuf[4].Position[1] = y2;
    vertexBuf[4].TexCoord[0] = tx;
    vertexBuf[4].TexCoord[1] = ty1;

    vertexBuf[5].Position[0] = x;
    vertexBuf[5].Position[1] = y1;
    vertexBuf[5].TexCoord[0] = tx;
    vertexBuf[5].TexCoord[1] = ty2;
}

void SetupGlyph(Vertex* vertexBuf, Font* font, int charIdx, float x, float y, float w, float h)
{
    const int col = charIdx % font->nCols;
    const int row = charIdx / font->nCols;
    const float tw = (1.0f / font->nCols);
    const float th = (1.0f / font->nRows);
    const float tx = col * tw;
    const float ty = row * th;
    const float ti = 0;
    SetupQuad(vertexBuf, x, y, w, h, tx+ti, ty+ti, tw-ti*2, th-ti*2);
}

void SetupString(Vertex* vertexBuf, Font* font, int* charIndices, int nCols, int nRows, float x, float y, float w, float h)
{
    const int nChars = nCols * nRows;
    const float charW = w / nCols;
    const float charH = h / nRows;
    for(int n = 0; n < nChars; n++)
    {
        SetupGlyph(&vertexBuf[n*6], font, charIndices[n], x+charW*(n%nCols), y+charH*(n/nCols), charW, charH);
    }
}

float gTime = 0.f;

void warp(float& x, float& y)
{
    float ox = x;
    float oy = y;
    //float d = sqrtf(ox*ox+oy*oy);
    float d = (ox*ox*oy*oy);
    const float warpAmt = 0.15f;
    x = ox + ox * d * -warpAmt + sinf(y*4. + gTime*15.) * 0.00015f;
    y = oy + oy * d * -warpAmt;
    x += (rand()%10)*0.00015f;
}

void SetupGrid(Vertex* vertexBuf, int nCols, int nRows, float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    const float tcw = tw / nCols;
    const float tch = th / nRows;
    const float cw = w / nCols;
    const float ch = h / nRows;
    for(int row = 0; row < nRows; row++)
    {
        for(int col = 0; col < nCols; col++)
        {
            float x1 = (x + col * cw);
            float y1 = (y + row * ch);
            float x4 = (x1 + cw);
            float y4 = (y1 + ch);
            float x2 = x4;
            float y2 = y1;
            float x3 = x1;
            float y3 = y4;
            float tx1 = tx + col * tcw;
            float ty1 = ty + row * tch;
            float tx4 = tx1 + tcw;
            float ty4 = ty1 + tch;
            float tx2 = tx4;
            float ty2 = ty1;
            float tx3 = tx1;
            float ty3 = ty4;
            warp(x1, y1);
            warp(x2, y2);
            warp(x3, y3);
            warp(x4, y4);
            SetupRawQuad(&vertexBuf[(nCols*row+col)*6],
               x1, y1, tx1, ty1,
               x2, y2, tx2, ty2,
               x3, y3, tx3, ty3,
               x4, y4, tx4, ty4);
        }
    }

}

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

typedef struct
{
    GLuint fbo;
    GLuint tex;
    GLuint width;
    GLuint height;
} renderTarget;

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

int main()
{
  // Initialize GLFW, and if it fails to initialize for any reason, print it out to STDERR.
    if (!glfwInit())
    {
        fprintf(stderr, "Failed initialize GLFW.");
        exit(EXIT_FAILURE);
    }

    const int screenW = 1024;
    const int screenH = 768;

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
    glfwSwapInterval(1);

    //printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

    glewExperimental = GL_TRUE;
    glewInit();

#define SHADER_LINE(x)  x "\n"

    static const char * vs_source[] =
    {
        //SHADER_LINE( "#version 110 core" )
        SHADER_LINE( "attribute vec2 Position;" )
        SHADER_LINE( "attribute vec2 TexCoord;" )
        SHADER_LINE( "void main(void) {" )
        SHADER_LINE( "    gl_Position = vec4(Position,0,1);" )
        SHADER_LINE( "    gl_TexCoord[0] = vec4(TexCoord,0,0);" )
        SHADER_LINE( "}" )
    };

    static const char * fs_source[] =
    {
        //SHADER_LINE( "#version 110 core" )
        SHADER_LINE( "uniform sampler2D tex;" )
        SHADER_LINE( "uniform vec4 color;" )
        SHADER_LINE( "void main(void) {" )
        SHADER_LINE( "    gl_FragColor = texture2D(tex, gl_TexCoord[0].xy) * color;" )
        //SHADER_LINE( "    gl_FragColor = vec4(1,1,1,1);" )
        SHADER_LINE( "}" )
    };


    int charIndices[rows][cols];
    Vertex vertexBuf[6 * cols * rows];

    for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            charIndices[r][c] = 0x7F;
        }
    }

    // const GLubyte Indices[] = {
    //     0, 1, 2,
    //     //2, 3, 0
    // };

    CHECK_GL_CALL( program = glCreateProgram() );

    CHECK_GL_CALL( GLuint fs = glCreateShader(GL_FRAGMENT_SHADER) );
    CHECK_GL_CALL( glShaderSource(fs, 1, fs_source, NULL) );
    glCompileShader(fs);
    CHECK_SHADER_ERROR(fs);

    CHECK_GL_CALL( GLuint vs = glCreateShader(GL_VERTEX_SHADER) );
    CHECK_GL_CALL( glShaderSource(vs, 1, vs_source, NULL) );
    glCompileShader(vs);
    CHECK_SHADER_ERROR(vs);

    CHECK_GL_CALL( glAttachShader(program, vs) );
    CHECK_GL_CALL( glAttachShader(program, fs) );

    glLinkProgram(program);
    CHECK_LINK_ERROR(program);

    CHECK_GL_CALL( GLuint positionSlot = glGetAttribLocation(program, "Position") );
    CHECK_GL_CALL( GLuint texCoordSlot = glGetAttribLocation(program, "TexCoord") );

    CHECK_GL_CALL( glEnableVertexAttribArray(positionSlot) );
    CHECK_GL_CALL( glEnableVertexAttribArray(texCoordSlot) );

    GLuint vertexBuffer;
    CHECK_GL_CALL( glGenBuffers(1, &vertexBuffer) );
    CHECK_GL_CALL( glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer) );
    CHECK_GL_CALL( glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuf), vertexBuf, GL_DYNAMIC_DRAW) );

    // GLuint indexBuffer;
    // CHECK_GL_CALL( glGenBuffers(1, &indexBuffer) );
    // CHECK_GL_CALL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer) );
    // CHECK_GL_CALL( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW) );


    CHECK_GL_CALL( glUseProgram(program) );

    CHECK_GL_CALL( glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,Position)) );
    CHECK_GL_CALL( glVertexAttribPointer(texCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,TexCoord)) );

    CHECK_GL_CALL( GLuint texLoc = glGetUniformLocation(program, "tex") );
    CHECK_GL_CALL( glUniform1i(texLoc, 0) );

    CHECK_GL_CALL( GLuint texColor = glGetUniformLocation(program, "color") );

    Font font;
    if(!LoadFont(&font, "PetASCII3.tga", 16, 16))
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

    CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );

    GLuint fontTex;
    CHECK_GL_CALL( glGenTextures(1, &fontTex) );
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, fontTex) );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    CHECK_GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, font.tga.imageWidth, font.tga.imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, font.tga.imageData) );

    GLuint bgTexID;
    CHECK_GL_CALL( glGenTextures(1, &bgTexID) );
    CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, bgTexID) );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    CHECK_GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgTga.imageWidth, bgTga.imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgTga.imageData) );

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

    glEnable(GL_BLEND);

    int frameIdx = 0;
    int cursor = 0;
    while(!glfwWindowShouldClose(window))
    {
        gTime = (frameIdx * 1./60);

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

        static int mode = 0;
        static int charSetLimit = 64;

        if(rand()%10 == 0) mode = rand()%10;

        if(rand()%20 == 0) charSetLimit = 32+(rand()%(256-32));

        
        if(mode == 0)
        {
            for(int cc = 0; cc < 1; cc++)
            {
                charIndices[rows-1][cursor % cols] = rand() % charSetLimit;
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
                unsigned int tmp = charIndices[y2][x2];
                charIndices[y2][x2] = charIndices[y1][x1];
                charIndices[y1][x1] = tmp;
            }
        }
        else if(mode == 2)
        {
            mode = 0;
            for(int r = 0; r < rows-1; r++)
            {
                for(int c = 0; c < cols; c++)
                {
                    charIndices[r][c] = charIndices[r+1][c];
                }
            }
            for(int c = 0; c < cols; c++)
            {
                charIndices[rows-1][c] = 0x7F;
            }
            cursor = 0;
        }

        /////////

        renderTarget_bind(&virtScreen);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SetupString(vertexBuf, &font, &charIndices[0][0], cols, rows, -1., -1., 2., 2.);

        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 1.f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, fontTex) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6*rows*cols) );

        renderTarget_finalize(&virtScreen);

        /////////

        renderTarget_bind(&phosphorLayer);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        {
            const float flickerA = 0.92f+0.08f*sinf(gTime*100.0f);
            const float scale = std::min( (float)screen.width/virtScreen.width, (float)screen.height/virtScreen.height) * 0.77f;
            const float sqX = (-scale*virtScreen.width)/screen.width;
            const float sqY = -(-scale*virtScreen.height)/screen.height;
            const float sqW = (2*scale*virtScreen.width)/screen.width;
            const float sqH = -(2*scale*virtScreen.height)/screen.height;
            SetupGrid(vertexBuf, 20, 20, sqX, sqY, sqW, sqH, 0, 1, 1, -1);
            //SetupQuad(vertexBuf, sqX, sqY, sqW, sqH, 0, 1, 1, -1);
            CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
            CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, flickerA) );
            CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
            CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
            CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, virtScreen.tex) );
            CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6*20*20) );
        }

        renderTarget_finalize(&phosphorLayer);

        /////////

        renderTarget_bind(&phosphorLayer2);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 1.0f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer.tex) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6) );

        renderTarget_finalize(&phosphorLayer2);

        /////////

        renderTarget_bind(&phosphorLayer3);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 1.0f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer2.tex) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6) );

        renderTarget_finalize(&phosphorLayer3);

        /////////

        renderTarget_bind(&screen);

        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 0.7f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, bgTexID) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6) );

        // {
        //     const float scale = std::min( (float)screen.width/virtScreen.width, (float)screen.height/virtScreen.height) * 0.77f;
        //     const float sqX = (-scale*virtScreen.width)/screen.width;
        //     const float sqY = -(-scale*virtScreen.height)/screen.height;
        //     const float sqW = (2*scale*virtScreen.width)/screen.width;
        //     const float sqH = -(2*scale*virtScreen.height)/screen.height;
        //     SetupGrid(vertexBuf, 20, 20, sqX, sqY, sqW, sqH, 0, 1, 1, -1);
        //     //SetupQuad(vertexBuf, sqX, sqY, sqW, sqH, 0, 1, 1, -1);
        //     CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        //     CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 0.7f) );
        //     CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        //     CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        //     CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, virtScreen.tex) );
        //     CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6*20*20) );
        // }

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 0.7f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer.tex) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6) );

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 0.1f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer2.tex) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6) );

        SetupQuad(vertexBuf, -1, -1, 2, 2, 0, 0, 1, 1);
        CHECK_GL_CALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        CHECK_GL_CALL( glUniform4f(texColor, 1.f, 1.f, 1.f, 0.2f) );
        CHECK_GL_CALL( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexBuf), vertexBuf) );
        CHECK_GL_CALL( glActiveTexture(GL_TEXTURE0) );
        CHECK_GL_CALL( glBindTexture(GL_TEXTURE_2D, phosphorLayer3.tex) );
        CHECK_GL_CALL( glDrawArrays(GL_TRIANGLES, 0, 6) );

        

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frameIdx;
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    return 0;
}

// [END]
