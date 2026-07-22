# OpenGL and GLSL — Complete Guide

Everything graphics: **OpenGL API + GLSL shaders** together — functions, properties, use cases, and examples.  
Engine-specific wrappers in this project are shown as *how we call the same ideas from C++*.

C++ language theory lives in a separate doc: [CPP_Fundamentals.md](CPP_Fundamentals.md)

---

## Table of contents

1. [Mental model](#1-mental-model)
2. [Setup: context, window, loaders](#2-setup-context-window-loaders)
3. [Coordinate spaces and MVP](#3-coordinate-spaces-and-mvp)
4. [GPU buffers and vertex layout](#4-gpu-buffers-and-vertex-layout)
5. [Shaders: GLSL language](#5-shaders-glsl-language)
6. [Shader programs and uniforms (OpenGL side)](#6-shader-programs-and-uniforms-opengl-side)
7. [Drawing and render state](#7-drawing-and-render-state)
8. [Textures and samplers](#8-textures-and-samplers)
9. [Lighting in GLSL](#9-lighting-in-glsl)
10. [Framebuffers and shadow mapping](#10-framebuffers-and-shadow-mapping)
11. [This project’s shaders walkthrough](#11-this-projects-shaders-walkthrough)
12. [Engine API map (optional)](#12-engine-api-map-optional)
13. [Frame recipe](#13-frame-recipe)
14. [Cheat sheets](#14-cheat-sheets)

---

## 1. Mental model

```text
CPU (your app)                         GPU
────────────────                       ────────────────────────────
Create window + GL context
Upload vertices              ──copy──► VBO (vertex bytes in VRAM)
Describe layout              ────────► VAO (how to read those bytes)
Compile GLSL                 ────────► Shader program
Each frame:
  set uniforms (matrices, lights)
  bind VAO + program
  glDraw*                    ────────► vertex shader → rasterize → fragment shader → framebuffer
```

| Piece | Role |
|-------|------|
| **OpenGL** | Commands: create buffers, compile shaders, draw |
| **GLSL** | Programs that run **on the GPU** per vertex / per pixel |
| **GLFW** | Window + context + input (not drawing) |
| **GLAD** | Loads OpenGL function pointers |
| **GLM** | CPU-side math (`vec3`, `mat4`) matching GLSL types |

---

## 2. Setup: context, window, loaders

### Why you need this

OpenGL draws into a **context**. No context → `gl*` calls crash or do nothing.

### GLFW functions (important)

| Function | What it does | Use case |
|----------|--------------|----------|
| `glfwInit()` | Start GLFW | Once at startup |
| `glfwWindowHint(...)` | Request GL version/profile | Before `glfwCreateWindow` |
| `glfwCreateWindow(w,h,title,...)` | Create window + context | Main viewport |
| `glfwMakeContextCurrent(win)` | Activate context for this thread | Before any `gl*` / GLAD |
| `glfwSwapBuffers(win)` | Show the frame you drew | End of every frame |
| `glfwPollEvents()` | Process OS input/messages | Every frame |
| `glfwWindowShouldClose(win)` | Exit condition | Main loop |
| `glfwGetKey(win, key)` | Keyboard state | Camera / quit |
| `glfwGetCursorPos(win, &x, &y)` | Mouse | Look controls |
| `glfwSetInputMode(win, GLFW_CURSOR, ...)` | Capture cursor | FPS camera |
| `glfwSetFramebufferSizeCallback` | Resize hook | Update `glViewport` |
| `glfwTerminate()` | Shutdown GLFW | App exit |

**Example — request OpenGL 3.3 Core**

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
GLFWwindow* window = glfwCreateWindow(800, 600, "App", nullptr, nullptr);
glfwMakeContextCurrent(window);
```

### GLAD

| Function | What it does |
|----------|--------------|
| `gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)` | Fill all `gl*` function pointers |

**Use case:** call immediately after `glfwMakeContextCurrent`.

### First OpenGL call after setup

| Function | Properties / args | Use case |
|----------|-------------------|----------|
| `glViewport(x, y, w, h)` | Pixel rectangle of the drawable | Match framebuffer size; call on resize |

```cpp
glViewport(0, 0, width, height);
```

---

## 3. Coordinate spaces and MVP

### Pipeline of a point

```text
Model space → World → View (camera) → Clip → NDC → Screen
   mesh          uModel    uView         uProjection
```

### GLSL vertex transform (standard)

```glsl
#version 330 core
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

layout (location = 0) in vec3 aPosition;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
```

| Matrix | Built on CPU (GLM) | Meaning |
|--------|--------------------|---------|
| Model | translate × rotate × scale | Object pose |
| View | `glm::lookAt(eye, center, up)` | Camera |
| Projection | `glm::perspective(fov, aspect, near, far)` | Lens / perspective |

**Use case:** every 3D object draw sets `uModel`; usually one `uView` / `uProjection` per camera per frame.

### Homogeneous `w`

`vec4(pos, 1.0)` for positions (translation applies).  
`vec4(dir, 0.0)` for directions (ignore translation).

---

## 4. GPU buffers and vertex layout

### Core OpenGL objects

| Object | OpenGL target / type | Stores |
|------|----------------------|--------|
| **VBO** | `GL_ARRAY_BUFFER` | Vertex attribute bytes |
| **EBO/IBO** | `GL_ELEMENT_ARRAY_BUFFER` | Indices (optional) |
| **VAO** | Vertex Array Object | Attribute format + which VBO |

### Functions

| Function | What it does | Use case |
|----------|--------------|----------|
| `glGenBuffers(n, &id)` | Create buffer name(s) | New VBO |
| `glBindBuffer(target, id)` | Make buffer active | Before upload/draw setup |
| `glBufferData(target, size, data, usage)` | Allocate + optional upload | Send mesh to GPU |
| `glDeleteBuffers(n, &id)` | Free GPU buffer | Cleanup / RAII dtor |
| `glGenVertexArrays` / `glBindVertexArray` | Create/bind VAO | Core profile draws |
| `glVertexAttribPointer(...)` | Describe one attribute | Position, normal, UV… |
| `glEnableVertexAttribArray(index)` | Turn attribute on | Must match shader `location` |
| `glDeleteVertexArrays` | Free VAO | Cleanup |

### `glBufferData` properties

| Arg | Meaning |
|-----|---------|
| `size` | Bytes to allocate/copy |
| `data` | CPU pointer (`nullptr` = allocate empty) |
| `GL_STATIC_DRAW` | Upload rarely, draw often (static mesh) |
| `GL_DYNAMIC_DRAW` | Change often |
| `GL_STREAM_DRAW` | Change every use |

**Example — triangle positions**

```cpp
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};
unsigned int vbo, vao;
glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glBindVertexArray(vao);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
```

### Interleaved layout (this project)

```text
[px py pz nx ny nz | px py pz nx ny nz | ...]
```

| Property | Example |
|----------|---------|
| Stride | `sizeof(Vertex)` (e.g. 24) |
| Position offset | `0` |
| Normal offset | `offsetof(Vertex, normal)` → often 12 |

**GLSL must match:**

```glsl
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
```

**Use case:** lit meshes need normals; UVs would be `location = 2` later.

---

## 5. Shaders: GLSL language

### Stages you use

| Stage | File | Runs | Main job |
|-------|------|------|----------|
| Vertex | `.vert` | Per vertex | Transform to clip space, pass varyings |
| Fragment | `.frag` | Per fragment (pixel candidate) | Output color |

Optional later: geometry, tessellation, compute.

### File header rule

```glsl
#version 330 core
```

Must be the **first** token in the file (no BOM junk, no duplicated `#version`).

### Types

| GLSL type | Meaning | Typical use |
|-----------|---------|-------------|
| `float` | Scalar | Shininess, attenuation |
| `vec2` | 2 floats | UVs |
| `vec3` | 3 floats | Position, normal, RGB |
| `vec4` | 4 floats | Clip pos, RGBA |
| `mat3` / `mat4` | Matrices | Normal matrix / MVP |
| `int` / `bool` | Integers / flags | Light enabled, counts |
| `sampler2D` | 2D texture | Albedo, shadow map |

### Qualifiers

| Qualifier | Meaning | Example |
|-----------|---------|---------|
| `in` | Input to this stage | `in vec3 aPosition;` |
| `out` | Output to next stage | `out vec3 vWorldNormal;` |
| `uniform` | Constant from CPU for the draw | `uniform mat4 uView;` |
| `const` | Compile-time constant | `const float PI = 3.14159;` |
| `layout(location = N)` | Attribute/binding index | Match VAO attrib `N` |

### Built-in variables

| Name | Stage | Meaning |
|------|-------|---------|
| `gl_Position` | Vertex out | Clip-space position (**required**) |
| `gl_FragCoord` | Fragment | Window-space fragment coordinate |
| `gl_FrontFacing` | Fragment | Front vs back face |

### Swizzling

```glsl
vec3 rgb = color.rgb;
vec2 xy = pos.xy;
vec3 flipped = pos.xzy;
```

### Common built-in functions

| Function | What it does | Use case |
|----------|--------------|----------|
| `normalize(v)` | Unit vector | Lighting directions |
| `dot(a,b)` | Dot product | Diffuse intensity `N·L` |
| `cross(a,b)` | Cross product | Tangent basis |
| `length(v)` | Magnitude | Distance to light |
| `distance(a,b)` | `length(a-b)` | Attenuation |
| `max/min/clamp` | Limit range | Avoid negative lighting |
| `mix(a,b,t)` | Lerp | Hemisphere sky/ground blend |
| `pow(x,n)` | Power | Specular shininess |
| `reflect/refract` | Optics | Advanced materials |
| `texture(sampler, uv)` | Sample texture | Color / shadow depth |
| `textureSize(s, lod)` | Resolution | PCF texel size |
| `inverse` / `transpose` | Matrix ops | Normal matrix |

### Structs and arrays

```glsl
struct Material {
    vec3 albedo;
    vec3 specularColor;
    float shininess;
};

uniform Material uMaterial;
uniform PointLight uPointLights[4];
uniform int uPointLightCount;
```

**Use case:** group related uniforms; C++ sets `uMaterial.albedo`, `uPointLights[0].position`, …

### Control flow

```glsl
if (enabled == 0) return vec3(0.0);

for (int i = 0; i < uPointLightCount; ++i) {
    color += shadePoint(uPointLights[i]);
}
```

Keep loops small; GPU prefers simple branching.

### Vertex shader template

```glsl
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel, uView, uProjection;

out vec3 vWorldPosition;
out vec3 vWorldNormal;

void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vWorldPosition = world.xyz;
    vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProjection * uView * world;
}
```

**Why inverse-transpose?** Non-uniform scale would skew normals if you used `mat3(uModel)` alone.

### Fragment shader template

```glsl
#version 330 core
in vec3 vWorldPosition;
in vec3 vWorldNormal;
out vec4 fragmentColor;

uniform vec3 uViewPosition;
uniform vec3 uAlbedo;

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(uViewPosition - vWorldPosition);
    fragmentColor = vec4(uAlbedo * max(dot(N, V), 0.0), 1.0); // toy lighting
}
```

### Depth-only fragment shader

```glsl
#version 330 core
void main() { }  // depth written automatically; no color
```

**Use case:** shadow map pass.

---

## 6. Shader programs and uniforms (OpenGL side)

### Compile / link functions

| Function | What it does | Use case |
|----------|--------------|----------|
| `glCreateShader(type)` | Create vertex/fragment shader object | `GL_VERTEX_SHADER` / `GL_FRAGMENT_SHADER` |
| `glShaderSource` | Attach GLSL source string(s) | Load from `.vert`/`.frag` files |
| `glCompileShader` | Compile on GPU driver | Check `GL_COMPILE_STATUS` |
| `glGetShaderiv` / `glGetShaderInfoLog` | Query errors | Print compile log |
| `glCreateProgram` | Create program | Link stages together |
| `glAttachShader` | Attach compiled stage | Vert + frag |
| `glLinkProgram` | Link into executable program | Check `GL_LINK_STATUS` |
| `glDeleteShader` | Free stage object | After successful link |
| `glUseProgram(id)` | Bind program for draws | Before uniforms + draw |
| `glDeleteProgram` | Free program | Shutdown / RAII |
| `glGetUniformLocation(prog, "name")` | Find uniform slot | Cache the int location |

**Example — compile check**

```cpp
int success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    char log[1024];
    glGetShaderInfoLog(shader, 1024, nullptr, log);
    // log contains the GLSL error line
}
```

### Setting uniforms

| Function | GLSL type | Example |
|----------|-----------|---------|
| `glUniform1i` | `int`, sampler unit | `glUniform1i(loc, 0);` |
| `glUniform1f` | `float` | shininess |
| `glUniform3f` / `3fv` | `vec3` | light color |
| `glUniform4f` / `4fv` | `vec4` | |
| `glUniformMatrix4fv` | `mat4` | MVP / light-space |

```cpp
glUseProgram(program);
GLint loc = glGetUniformLocation(program, "uModel");
glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
```

| Property | Meaning |
|----------|---------|
| `count` | Number of matrices/vectors |
| `transpose` | `GL_FALSE` for GLM (column-major) |

**Use case:** if location is `-1`, name is wrong or optimized out — uniform silently does nothing.

### Sampler uniforms

```glsl
uniform sampler2D uDirectionalShadowMap;
```

```cpp
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, depthTex);
glUniform1i(glGetUniformLocation(prog, "uDirectionalShadowMap"), 0); // unit 0
```

The uniform stores the **texture unit index**, not a pointer.

---

## 7. Drawing and render state

### Draw calls

| Function | What it does | Use case |
|----------|--------------|----------|
| `glDrawArrays(mode, first, count)` | Draw `count` vertices from VAO | Non-indexed mesh (this project’s cube) |
| `glDrawElements(mode, count, type, indices)` | Indexed draw | Shared vertices / OBJ meshes |

Modes: `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, `GL_LINES`, …

```cpp
glUseProgram(program);
glBindVertexArray(vao);
glDrawArrays(GL_TRIANGLES, 0, 36); // cube = 36 verts
```

### Clear and color

| Function | What it does |
|----------|--------------|
| `glClearColor(r,g,b,a)` | Set clear color |
| `glClear(mask)` | Clear buffers |

```cpp
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

### Depth testing

| Function / enum | Meaning |
|-----------------|---------|
| `glEnable(GL_DEPTH_TEST)` | Closer fragments win |
| `glDepthFunc(GL_LESS)` | Default comparison |
| `GL_DEPTH_BUFFER_BIT` | Clear depth each frame |

**Use case:** any real 3D scene. Without it, draw order alone decides visibility.

### Face culling (optional optimization)

```cpp
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
```

**Use case:** skip back faces of closed meshes (careful with scaled-negative meshes).

### Polygon offset (shadow helpers)

```cpp
glEnable(GL_POLYGON_OFFSET_FILL);
glPolygonOffset(2.0f, 4.0f);
```

**Use case:** reduce shadow acne when rendering the depth pass.

---

## 8. Textures and samplers

Even if you only use a **depth texture** for shadows today, the same API applies to color textures later.

| Function | What it does | Use case |
|----------|--------------|----------|
| `glGenTextures` | Create texture name | Albedo / shadow map |
| `glBindTexture(GL_TEXTURE_2D, id)` | Bind 2D texture | Upload / sample |
| `glTexImage2D(...)` | Allocate + optional pixels | Color or depth storage |
| `glTexParameteri` | Wrap/filter mode | Repeat, clamp, nearest/linear |
| `glActiveTexture(GL_TEXTUREn)` | Select unit `n` | Multi-texture |
| `glDeleteTextures` | Free | Cleanup |

### Depth texture example (shadow map)

```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
             width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
float border[] = {1,1,1,1}; // outside = "lit"
glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
```

### Sampling in GLSL

```glsl
float depth = texture(uDirectionalShadowMap, uv).r;
vec4  albedo = texture(uAlbedo, uv);
```

---

## 9. Lighting in GLSL

### Blinn-Phong (what this project uses)

| Term | Idea | GLSL sketch |
|------|------|-------------|
| Diffuse | Surface faces light | `max(dot(N, L), 0) * albedo` |
| Specular | Highlight toward view | `pow(max(dot(N, H), 0), shininess)` |
| Ambient / hemisphere | Fill light | Sky/ground mix by `N.y` |

Halfway vector: `H = normalize(L + V)`.

### Light types

| Type | CPU data | GLSL behavior |
|------|----------|---------------|
| **Directional** | direction, color, intensity | Parallel rays; no distance falloff |
| **Point** | position + attenuation | Omnidirectional; `1/(c + l*d + q*d²)` |
| **Spot** | position, direction, cutoffs | Cone; smooth inner/outer edge |
| **Hemisphere** | sky/ground colors | Cheap indirect fill |
| **Floor bounce** | bounce color + height | Fake bounce (not real GI) |

### Directional light vector

Light direction uniform often points **toward the scene** (down).  
Lighting uses `L = normalize(-uDirectional.direction)` (from surface toward the sun).

### Spot cone

Upload **cosines** of angles from CPU:

```cpp
cos(innerRadians), cos(outerRadians)
```

```glsl
float theta = dot(L, normalize(-spotDir));
float cone = clamp((theta - outer) / max(inner - outer, 0.0001), 0.0, 1.0);
```

### Tone mapping + gamma (final color)

```glsl
vec3 mapped = lighting / (lighting + vec3(1.0));   // Reinhard-style
vec3 srgb = pow(mapped, vec3(1.0 / 2.2));
fragmentColor = vec4(srgb, 1.0);
```

**Use case:** keep bright lights from blowing out to pure white clipping without control.

---

## 10. Framebuffers and shadow mapping

### Framebuffer objects (FBO)

| Function | What it does |
|----------|--------------|
| `glGenFramebuffers` | Create FBO |
| `glBindFramebuffer(GL_FRAMEBUFFER, fbo)` | Render into FBO (0 = default screen) |
| `glFramebufferTexture2D` | Attach texture as color/depth |
| `glCheckFramebufferStatus` | Must be `GL_FRAMEBUFFER_COMPLETE` |
| `glDrawBuffer(GL_NONE)` / `glReadBuffer(GL_NONE)` | Depth-only FBO (no color) |

### Shadow map algorithm

```text
1. Render scene from light → depth texture (light’s depth)
2. Render scene from camera
3. For each fragment, project into light space
4. Compare fragment depth vs shadow map depth → shadowed or lit
```

### Light-space matrix (directional)

```text
lightView = lookAt(center - dir * distance, center, up)
lightProj = ortho(-half, half, -half, half, near, far)
lightSpace = lightProj * lightView
```

Vertex (depth pass):

```glsl
gl_Position = uLightSpaceMatrix * uModel * vec4(aPosition, 1.0);
```

Vertex (lit pass also outputs):

```glsl
vLightSpacePosition = uLightSpaceMatrix * worldPosition;
```

### PCF soft shadows (fragment)

```glsl
vec3 proj = vLightSpacePosition.xyz / vLightSpacePosition.w;
proj = proj * 0.5 + 0.5;
float bias = max(0.0025 * (1.0 - dot(N, L)), 0.0005);
float shadow = 0.0;
vec2 texel = 1.0 / textureSize(uShadowMap, 0);
for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y) {
        float closest = texture(uShadowMap, proj.xy + vec2(x,y)*texel).r;
        shadow += (proj.z - bias > closest) ? 1.0 : 0.0;
    }
shadow /= 9.0;
// apply: radiance *= (1.0 - shadow);
```

| Issue | Mitigation |
|-------|------------|
| Shadow acne | Bias + polygon offset |
| Peter-panning | Bias too large |
| Hard edges | PCF / higher resolution |
| Outside map | Clamp border to 1.0 (fully lit) |

---

## 11. This project’s shaders walkthrough

### `Assets/Shaders/Lit.vert`

| Item | Role |
|------|------|
| `aPosition`, `aNormal` | Mesh attributes |
| `uModel/View/Projection` | Camera transform |
| `uLightSpaceMatrix` | Shadow projection |
| `vWorldPosition/Normal` | Lighting in world space |
| `vLightSpacePosition` | Shadow test coords |

### `Assets/Shaders/Lit.frag`

| Function | Role |
|----------|------|
| `evaluateSurfaceLight` | Diffuse + Blinn specular |
| `evaluateDirectional` | Sun × (1 − shadow) |
| `evaluatePoint` | Point lights |
| `evaluateSpot` | Camera flashlight |
| `evaluateIndirect` | Hemisphere + floor bounce |
| `evaluateDirectionalShadow` | PCF |
| `main` | Sum → tone map → gamma |

### `Assets/Shaders/ShadowDepth.vert` / `.frag`

Depth-only pass into the shadow FBO.

---

## 12. Engine API map (optional)

Thin C++ wrappers around the ideas above (see source under `Engine/`):

| Engine class | Wraps |
|--------------|-------|
| `Window` | GLFW + GLAD + viewport |
| `VertexBuffer` / `VertexArray` | VBO / VAO |
| `Mesh` / `PrimitiveFactory` | Interleaved cube mesh |
| `Shader` | Compile/link + `setMat4`/`setVec3`… |
| `Renderer` | Clear, depth, `glDrawArrays` |
| `Transform` / `PerspectiveCamera` | Model / view / projection matrices |
| `LightingEnvironment` | Upload light uniforms |
| `ShadowMap` / `DirectionalShadowPass` | FBO depth + light-space pass |

Use this guide for **OpenGL/GLSL meaning**; use source files for **exact C++ signatures**.

---

## 13. Frame recipe

```text
update camera / object transforms
── Shadow pass ──
  bind shadow FBO, viewport = shadow resolution
  clear depth
  use depth shader; set lightSpace
  draw casters
  bind FBO 0; restore viewport
── Lit pass ──
  clear color+depth
  use lit shader; set view/projection/lightSpace
  bind shadow texture; set lights/materials
  draw objects
swap buffers; poll events
```

---

## 14. Cheat sheets

### OpenGL must-know calls

```text
Context:    glfw* + gladLoadGLLoader + glViewport
Data:       glGenBuffers, glBufferData, glVertexAttribPointer, VAO
Shaders:    create/compile/attach/link/use + glUniform*
Draw:       glClear, glEnable(GL_DEPTH_TEST), glDrawArrays
Textures:   glTexImage2D, glActiveTexture, sampler uniform
Offscreen:  FBO + glFramebufferTexture2D
```

### GLSL must-know

```text
#version 330 core
layout(location=N) in ...
uniform ...
out ... / in ... between stages
gl_Position in vertex main
out vec4 color in fragment main
texture(sampler, uv)
```

### Debug checklist (black screen)

1. Context created? GLAD loaded?  
2. Shader compile/link logs clean?  
3. Uniform names match exactly?  
4. VAO attribute locations match shader?  
5. Camera looking at the object? Near/far sane?  
6. Depth test on and depth cleared?  
7. Working directory finds `Assets/Shaders/...`?

---

*OpenGL issues commands. GLSL defines what the GPU computes. Learn them as one pipeline, not two isolated topics.*
