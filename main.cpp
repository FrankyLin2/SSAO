#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_s.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "graham.hpp"
#include "annotation.hpp"

#include <iostream>
#include <random>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderQuad();
void renderFloor();
void RenderMainImGui(GLFWwindow* window);
void genSsaoKernel(std::vector<glm::vec3> &ssaoKernel);
void updateModelMatrices(std::vector<glm::mat4> &modelMatrices, int maxAmount = 50, float posStddev = 2.0, float scaleMean = 1.0f, float scaleStddev = 0.1);
bool findSame(glm::vec4 &key, vector<glm::vec4> &elems);
GLfloat sampleMax(GLfloat *positionData, int x, int y);
// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;
//缓冲尺寸，对retina来说是两倍窗口尺寸
int BUF_WIDTH, BUF_HEIGHT;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH  / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

//partical distrubution
struct distributionConfig
{
    int shapeCurrent = 0;
    int maxAmount = 50;
    float posStddev = 2.0;
    float scaleMean = 1.0;
    float scaleStddev =0.1;
    bool flag = 0;
    bool save = 0;
}disConfig;

//shader config
struct shaderConfig
{
    float ka = 0.25;
    float kd = 0.5;
}shaderConfig;

//define color
vector<glm::vec3> colors{glm::vec3{0.0/255, 128.0/255, 128.0/255}, 
                        glm::vec3{128.0/255, 0.0/255, 128.0/255},
                        glm::vec3{75.0/255, 0/255, 128.0/255},
                        glm::vec3{0/255, 0/255, 128.0/255},
                        glm::vec3{20.0/255, 70.0/255, 150.0/255},
                        glm::vec3{255/255, 255/255, 0/255},
                        glm::vec3{255/255, 20.0/255, 60.0/255}};
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}

annotationWriter annoWriter;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    const char* glsl_version = "#version 330";
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &BUF_WIDTH, &BUF_HEIGHT);
    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // pass when depth is smaller (same effect as glDisable(GL_DEPTH_TEST))
    // glEnable(GL_STENCIL_TEST);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)一共包含三个选项，我们能够设定每个选项应该采取的行为：

    // sfail：模板测试失败时采取的行为。GL_KEEP
    // dpfail：模板测试通过，但深度测试失败时采取的行为。GL_KEEP
    // dppass：模板测试和深度测试都通过时采取的行为。GL_REPLACE 将模板值设置为glStencilFunc函数设置的ref值
    // build and compile shaders
    // -------------------------
    // Shader asteroidShader("../10.3.asteroids.vs", "../10.3.asteroids.fs");
    // Shader planetShader("../10.3.planet.vs", "../10.3.planet.fs");
    // Shader shaderGeometryPass("../../9.ssao_geometry.vs", "../../9.ssao_geometry.fs");
    // Shader shaderLightingPass("../../9.ssao.vs", "../../9.ssao_lighting.fs");
    // Shader shaderSSAO("../../9.ssao.vs", "../../9.ssao.fs");
    // Shader shaderSSAOBlur("../../9.ssao.vs", "../../9.ssao_blur.fs");
    // Model octahedron("../../asset/Octahedron.obj");
    // Model cube("../../asset/cube.obj");

    Shader shaderGeometryPass("../ssao_geometry.vs", "../ssao_geometry.fs");
    Shader shaderGeometryPass_floor("../ssao_geometry.vs", "../ssao_geometry_floor.fs");
    Shader shaderLightingPass("../ssao.vs", "../ssao_lighting.fs");
    Shader shaderSSAO("../ssao.vs", "../ssao.fs");
    Shader shaderSSAOBlur("../ssao.vs", "../ssao_blur.fs");

    auto octahedron = make_shared<Model>("../asset/Octahedron.obj");
    auto cube = make_shared<Model>("../asset/cube.obj");
    auto corner_truncated_cubes = make_shared<Model>("../asset/corner_truncated_cubes.obj");
    auto cuboctahedrons = make_shared<Model>("../asset/cuboctahedrons.obj");
    auto hexagon_octahedrons = make_shared<Model>("../asset/hexagon_octahedrons.obj");
    auto vertex_truncated_octahedrons = make_shared<Model>("../asset/vertex_truncated_octahedrons.obj");
    vector<shared_ptr<Model>> models{octahedron, cube, corner_truncated_cubes, cuboctahedrons, hexagon_octahedrons, vertex_truncated_octahedrons};

    // configure g-buffer framebuffer
    // ------------------------------
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    //bind framebuffer with gBuffer, then render to gBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedo, gDepth;//three G-buffer, 位置，法线，漫反射created in SSAO_geometry pass, saved as textures. 
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BUF_WIDTH, BUF_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BUF_WIDTH, BUF_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BUF_WIDTH, BUF_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
    //depth for discard vertex
    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, BUF_WIDTH, BUF_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gDepth, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);
    // create and attach depth buffer (renderbuffer), we don't need to sample depth later, so use renderbuffer,make it quicker
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, BUF_WIDTH, BUF_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // also create framebuffer to hold SSAO processing stage 
    // -----------------------------------------------------
    unsigned int ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BUF_WIDTH, BUF_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BUF_WIDTH, BUF_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate sample kernel
    // ----------------------
    std::vector<glm::vec3> ssaoKernel;
    genSsaoKernel(ssaoKernel);
    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    std::vector<glm::mat4> modelMatrices;
    updateModelMatrices(modelMatrices);
    // generate noise texture
    // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // lighting info
    // -------------
    glm::vec3 lightDir = glm::vec3(0.0, 0.0, -1.0);
    glm::vec3 lightColor = glm::vec3(1.0);

    // shader configuration
    // --------------------
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedo", 2);
    shaderLightingPass.setInt("ssao", 3);
    shaderLightingPass.setInt("gDepth", 4);
    shaderSSAO.use();

    shaderSSAO.setInt("gPosition", 0);
    shaderSSAO.setInt("gNormal", 1);
    shaderSSAO.setInt("texNoise", 2);
    shaderSSAOBlur.use();
    shaderSSAOBlur.setInt("ssaoInput", 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        
        // per-frame time logic
        // --------------------
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 50.0f);
        glm::mat4 projection = glm::ortho(-12.0f/2 * (float)SCR_WIDTH / (float)SCR_HEIGHT, 12.0f/2 * (float)SCR_WIDTH / (float)SCR_HEIGHT, -12.0f/2, 12.0f/2, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        shaderGeometryPass_floor.use();
        shaderGeometryPass_floor.setMat4("projection", projection);
        shaderGeometryPass_floor.setMat4("view", view);
        // renderFloor
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -30.0f));
        model = glm::scale(model, glm::vec3(15.0f, 15.0f, 15.0f));
        shaderGeometryPass_floor.setMat4("model", model);
        shaderGeometryPass_floor.setInt("invertedNormals", 1); // invert normals as we're inside the cube
        renderFloor();
        shaderGeometryPass_floor.setInt("invertedNormals", 0); 


        shaderGeometryPass.use();
        shaderGeometryPass.setMat4("projection", projection);
        shaderGeometryPass.setMat4("view", view);
        // octahedron model on the floor
        
        int colorInd = 0;
        for (auto modelMatrice: modelMatrices)
        {
            shaderGeometryPass.setMat4("model", modelMatrice);
            shaderGeometryPass.setVec3("Albedo", colors[colorInd%7]);
            colorInd++;
            models[disConfig.shapeCurrent]->Draw(shaderGeometryPass);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 2. generate SSAO texture
        // ------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAO.use();
            // Send kernel + rotation 
            for (unsigned int i = 0; i < 64; ++i)
                shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
            shaderSSAO.setMat4("projection", projection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 3. blur SSAO texture to remove noise
        // ------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAOBlur.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
        // -----------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.use();
        // send light relevant uniforms
        glm::vec3 lightDirView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightDir, 0.0));
        shaderLightingPass.setVec3("light.direction", lightDirView);
        shaderLightingPass.setVec3("light.Color", lightColor);
        // Update attenuation parameters
        const float linear    = 0.09f;
        const float quadratic = 0.032f;
        shaderLightingPass.setFloat("light.ka", shaderConfig.ka);
        shaderLightingPass.setFloat("light.kd", shaderConfig.kd);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gDepth);
        renderQuad();
        
        
        
        //保存图片,生成标注，更新分布
        if(disConfig.flag){
            //保存位置纹理，主要是为了b也就是深度z
            GLfloat *positionData = new GLfloat[BUF_HEIGHT*BUF_WIDTH];
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, positionData); 

            cv::Mat img(BUF_HEIGHT, BUF_WIDTH, CV_8UC3);
            glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);

            //模型中点的坐标
            std::vector<glm::vec4> verticesPos;
            auto vp = projection*view;
            for(auto &vertice: models[disConfig.shapeCurrent]->meshes[0].vertices){
                auto vert = glm::vec4(vertice.Position, 1.0f);
                //去掉重复的
                if(findSame(vert, verticesPos))
                    continue;
                verticesPos.push_back(vert);
            }
                
            
            //对每一个变换后的晶体模型，其所有点的mvp都是一样的
            for(auto &model: modelMatrices){
                //确定当前晶体mvp
                auto mvp = vp*model;
                vector<Point> points;
                int count = 0;
                //当前晶体所有点MVP变换到NDC,不用除以w因为是正交不是透视,然后由NDC变换到screen space，并把x，y坐标塞入点集
                for(auto vert: verticesPos){
                    vert = mvp * vert;
                    static Point pointxy;
                    pointxy.x = 0.5 * BUF_WIDTH * (vert.x + 1.0);
                    pointxy.y = 0.5 * BUF_HEIGHT * (vert.y + 1.0);
                    // cout<<(vert.z+1)/2<<"\t"<<positionData[pointxy.y*BUF_WIDTH + pointxy.x]<<endl;
                    //如果该点深度<该点附近最深的那个缓存深度，则该点可见，count++, 如果该点不在深度图中，即放弃该晶体，总不能裁剪吧哈哈哈太麻烦
                    auto minDepth = sampleMax(positionData, pointxy.x, pointxy.y);
                    if(minDepth < 0){
                        points.clear();
                        count = 0;
                        break;
                    }
                    if((vert.z+1)/2 < minDepth+0.001) count++;
                    points.push_back(pointxy);

                }
                //如果可见点数量小于所有点数量的四分之一，弃掉（完全可见应该接近并大于1/2)

                // cout<<verticesPos.size()<<"\t"<<count<<endl;
                if(count < 3) continue;;
                
                //求凸包，即最大外接多边形
                ConvexHull ch(points);
                ch.run();
                vector<Point> result = ch.getResult();
                // TODO：shape要可选择的，以后改
                annoWriter.addPolygon(result);
                //让端点显红色
                // for(auto resPoint: result){
                //     img.at<cv::Vec3b>(resPoint.y,resPoint.x) = cv::Vec3b(0,0,255);
                // }
            }
            //保存图片
            // cv::Mat flipped;
            //opengl纹理坐标与图片坐标系不同，opengl左下角为起点，图片左上角起点,理论上需要y轴翻转，但是没必要，我们的图不翻转也行
            // cv::flip(img, flipped, 0);
            cv::Mat result;
            cv::blur(img, result, cv::Size(5, 5));
            auto imgName = "result" + std::to_string(currentFrame);
            cv::imwrite(imgName + ".jpg", result);
            //深度图
            // cv::Mat tex(BUF_HEIGHT, BUF_WIDTH, CV_32FC1, positionData);
            // cv::flip(tex, tex, 0);
            // cv::imshow("tex", tex);
            //生成标签
            annoWriter.genAnnotation(imgName);
            
            updateModelMatrices(modelMatrices, disConfig.maxAmount, disConfig.posStddev, disConfig.scaleMean, disConfig.scaleStddev);
            // disConfig.flag = 0;
            delete [] positionData;
        }
        if(disConfig.save){
            annoWriter.writeToFile("annotation.json");
            disConfig.save = 0;
        }
        RenderMainImGui(window);
        


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    return 0;
}

// renderFloor() renders floor.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderFloor()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
  
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
// void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
// {
//     float xpos = static_cast<float>(xposIn);
//     float ypos = static_cast<float>(yposIn);
//     if (firstMouse)
//     {
//         lastX = xpos;
//         lastY = ypos;
//         firstMouse = false;
//     }

//     float xoffset = xpos - lastX;
//     float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

//     lastX = xpos;
//     lastY = ypos;

//     camera.ProcessMouseMovement(xoffset, yoffset);
// }

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
void RenderMainImGui(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("ImGui Window");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("maxAmount: max number of cubes. default 50");
        ImGui::SliderInt("maxAmount", &disConfig.maxAmount,10,200);
        ImGui::SliderFloat("posStddev",&disConfig.posStddev,1.0f,5.0f);
        ImGui::SliderFloat("scaleMean",&disConfig.scaleMean,0.2f,2.0f);
        ImGui::SliderFloat("scaleStddev",&disConfig.scaleStddev,0.05f,0.25f);
        
        ImGui::SliderFloat("ka:ambient",&shaderConfig.ka,0.0f,0.8f);
        ImGui::SliderFloat("kd:diffusion",&shaderConfig.kd,0.0f,1.0f);
        
        ImGui::Combo("Shape", &disConfig.shapeCurrent, "octahedron\0cube\0corner_truncated_cubes\0cuboctahedrons\0hexagon_octahedrons\0vertex_truncated_octahedrons\0\0");
        //重新生成图片
        if (ImGui::Button("generate")){
            disConfig.flag = 1;
        }
        if(ImGui::Button("stop")){
            disConfig.flag = 0;
        }
        if(ImGui::Button("save")){
            disConfig.save = 1;
        }
        //自定义GUI内容
        ImGui::End();        
    }
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void genSsaoKernel(std::vector<glm::vec3> &ssaoKernel){
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator(rd());
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0);
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        // scale = ourLerp(0.1f, 1.0f, scale * scale);
        // sample *= scale;
        ssaoKernel.push_back(sample);
    }
}
    //posStddev: stddev of model's position distribution, default 2.0
    //scaleMean: average size, default 1.0;
    //scaleStddev: makes scale between (1-3*stddev, 1+3*stddev),default 0.1
    //maxAmount: max number of cubes. default 50
void updateModelMatrices(std::vector<glm::mat4> &modelMatrices, int maxAmount, float posStddev, float scaleMean, float scaleStddev){
    
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::normal_distribution<GLfloat> normalRandomFloats(0.0, 1.0);
    std::random_device rd;
    std::default_random_engine generator(rd());
    //amount random from 0-50;
    unsigned int amount = randomFloats(generator) * maxAmount;
    //mean:center of model's position distribution
    glm::vec3 mean = glm::vec3(normalRandomFloats(generator), normalRandomFloats(generator), normalRandomFloats(generator) * 2 - 20);
    modelMatrices = std::vector<glm::mat4>(amount);
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: 
        glm::vec3 pos = glm::vec3(normalRandomFloats(generator) * posStddev, normalRandomFloats(generator) * posStddev, randomFloats(generator) * 20-10) + mean;
        model = glm::translate(model, pos);

        // 2. scale: Scale between 0.7 and 1.3f
        float scale = normalRandomFloats(generator) * scaleStddev + 1.0;
        model = glm::scale(model, glm::vec3(scaleMean));
        model = glm::scale(model, glm::vec3(scale));
        // 3. rotation: add random rotation around three rotation axis vector
        model = glm::rotate(model, randomFloats(generator) * 360.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, randomFloats(generator) * 360.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, randomFloats(generator) * 360.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }
}
bool findSame(glm::vec4 &key, vector<glm::vec4> &elems){
    for(auto elem: elems){
        if(glm::all(glm::equal(key,elem)))
        return true;
    }
    return false;
}
//从深度图x，y点返回周围九格深度最大点，（0，1）之间，如不存在返回-1
GLfloat sampleMax(GLfloat *positionData, int x, int y){
    if(y < 0 || x < 0 || y > BUF_HEIGHT - 1 || x > BUF_WIDTH - 1) return -1.0;

    if(y <= 0) y=1;
    if(x <= 0) x=1;
    if(y >= BUF_HEIGHT - 1) y=BUF_HEIGHT - 2;
    if(x >= BUF_WIDTH - 1) x= BUF_WIDTH - 2;

    return max({positionData[(y-1)*BUF_WIDTH + x-1],positionData[(y-1)*BUF_WIDTH +x],positionData[(y-1)*BUF_WIDTH + x+1],
        positionData[(y)*BUF_WIDTH + x-1],positionData[(y)*BUF_WIDTH + x],positionData[(y)*BUF_WIDTH + x+1],
        positionData[(y+1)*BUF_WIDTH + x-1],positionData[(y+1)*BUF_WIDTH + x],positionData[(y+1)*BUF_WIDTH + x+1]});

}