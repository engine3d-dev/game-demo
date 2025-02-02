#include "LevelScene.hpp"
#include "core/application_instance.hpp"
#include <core/engine_logger.hpp>
#include <core/event/key_codes.hpp>
#include <core/update_handlers/sync_update.hpp>
#include <core/event/input_poll.hpp>
// #include <imgui.h>
#include <flecs/addons/cpp/mixins/meta/cursor.hpp>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <renderer/renderer.hpp>
#include <core/ui/widgets.hpp>
#include <scene/components/components.hpp>
#include <string>
#include <vulkan/vulkan_core.h>
#include <drivers/vulkan/helper_functions.hpp>
#include <core/image/stb_image.hpp>
#include <core/math/math.hpp>

constexpr int ROTATION_DIRECTION_X = -1;
constexpr int ROTATION_DIRECTION_Y = -1;
constexpr float PULL_WEIGHT = 0.01f;

namespace engine3d{
    struct MeshData{
        glm::vec3 Position{0.f};
        glm::vec3 Scale{0.f};
        glm::vec3 Rotation{0.f};
        std::string mesh_file = "";
    };

    struct CameraData{
        glm::vec3 Position{0.f};
        glm::vec3 Front{0.f};
        float Angle = 90.f;
    };

    static VkSampler g_TextureSampler;

    struct VulkanTextureData{
        VkImage Image;
        VkImageView ImageView;
        VkSampler Sampler;
    };

    // static VulkanTextureData g_Texture;

    static std::vector<VkImageView> g_ViewportImages;

    static float sensitivity = 0.f;

    static MeshData sphere_data;
    static MeshData some_mesh_data;
    static CameraData camera_data;
    static std::string g_MeshFilepath = "";
    // static float angle = glm::radians(45.0f);

    struct Projectile{
        glm::vec3 Position;
    };

    LevelScene::LevelScene(const std::string& p_Tag) : SceneScope(p_Tag){
        ConsoleLogInfo("SceneScope::SceneScope with Tag = {} called!", p_Tag);
        WorldScope world = SystemRegistry::GetWorld();
        std::string world_tag = world.GetTag();
        ConsoleLogTrace("From LevelScene::LevelScene() ==> World Tag = {}", world_tag);

        // Renderer::OverrideShader("filepath/custom.vs", "filepath/custom.fs", true);

        //! @note Creating our objects from our scene
        m_Sphere = this->CreateNewObject("sphere");
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/castelia city.obj"});
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/Castelia City.obj"});
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/E 45 Aircraft_obj.obj"});
        // sphere_data.mesh_file = "3d_models/tutorial/FinalBaseMesh.obj";
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/FinalBaseMesh.obj"});
        m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/Ball OBJ.obj"});
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/quad.obj"});
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/sphere.obj"});
        m_Sphere->SetComponent<Transform>({
            .Position = {0.f, 2.10f, -7.30f},
            .Scale = {.20f,.20f, .20f}
        });

        for (size_t i = 0; i < 5 - 1; ++i) 
        {
                lengths.push_back(1.5f);
        }

        auto transform = m_Sphere->GetComponent<Transform>();

        sphere_data.Position = transform->Position;
        sphere_data.Scale = transform->Scale;
        sphere_data.Rotation = transform->Rotation;

        m_SomeMesh = this->CreateNewObject("mesh1");
        m_SomeMesh->SetComponent<Transform>({
            .Position = {0.f, 1.40f, -7.4f},
            .Scale = {2.80f, 1.f, 3.70f}
        });

        auto some_mesh_transform = m_SomeMesh->GetComponent<Transform>();
        some_mesh_data.Position = some_mesh_transform->Position;
        some_mesh_data.Scale = some_mesh_transform->Scale;
        some_mesh_data.Rotation = some_mesh_transform->Rotation;

        // m_SomeMesh->SetComponent<MeshComponent>({"3d_models/tutorial/sphere.obj"});
        // sphere_data.mesh_file = "3d_models/tutorial/quad.obj";
        m_SomeMesh->SetComponent<MeshComponent>({"3d_models/tutorial/quad.obj"});

        m_Camera = this->CreateNewObject("camera");

        camera_data.Position = {0.0f, 1.50f, 0.0f};
        camera_data.Front = glm::vec3(-0.0f, 0.0f, -1.0f);

        m_Camera->AddComponent<PerspectiveCamera>();

        sensitivity = m_Camera->GetComponent<PerspectiveCamera>()->MovementSpeed;
        // g_TextureSampler = CreateSampler();

        // g_DescriptorSets.resize(ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImagesSize());
        // ConsoleLogTrace("g_DescriptorSets.size() = {}", g_DescriptorSets.size());
        // for(size_t i = 0; i < g_DescriptorSets.size(); i++){
        //     g_DescriptorSets[i] = ImGui_ImplVulkan_AddTexture(g_TextureSampler, ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImageView(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        // }

        m_MoreObjects.resize(1000);

        
        MeshComponent mesh = {"3d_models/tutorial/sphere.obj"};
        
        int counter = 0;
        for(size_t x= 0; x < 10; x++)
        {
            for(size_t y= 0; y < 10; y++)
            {
                for(size_t z = 0; z < 10; z++)
                {
                    
                    m_MoreObjects[counter] = this->CreateNewObject(fmt::format("Object {}", counter));
                    
                    m_MoreObjects[counter]->AddComponent<MeshComponent>(mesh);
                    if(counter == 5)
                        m_MoreObjects[counter]->SetComponent<MeshComponent>({"3d_models/tutorial/quad.obj"});
                    m_MoreObjects[counter]->SetComponent<Transform>({
                        .Position = {counter,0,0},
                        .Scale = {.10f,.10f, .10f}
                    });
                    m_SceneObjectLookup.insert({counter, m_MoreObjects[counter]});
                    counter++;
                }
            }
        }

        m_MoreObjects[0]->SetComponent<Transform>({
            .Position = {
                -12.7,
                6.7,
                0
                
            },
            .Scale = {.10f,.10f, .10f}
        });

        m_MoreObjects[5]->SetComponent<Transform>({
            .Position = {
                -9.7,
                3.3,
                0
            },
            .Scale = {1.0f,1.0f, 1.0f}
        });

        m_MoreObjects[6]->SetComponent<Transform>({
            .Position = {
                -10.9,
                13.3,
                0
            },
            .Scale = {.0010f,.0010f, .0010f}
        });

        SyncUpdate::Subscribe(this, &LevelScene::OnUpdate);
        // SyncUpdate::Subscribe(this, &LevelScene::PhysicsUpdate);
        SyncUpdate::SubscribeOnRender(this, &LevelScene::OnSceneRender);
        SyncUpdate::ConnectWidget(this, &LevelScene::OnUIUpdate);
        // SyncUpdate::Subscribe(this, &LevelScene::OnSceneRender);
        // sync_update::Subscribe(this, &LevelScene::OnUpdate);
        // sync_update::ConnectWidget(this, &LevelScene::OnUIUpdate);/
    }

    void LevelScene::OnUIUpdate(){
        // ConsoleLogTrace("OnUIUpdate called!");
        // DockspaceWindow(ApplicationInstance::GetWindow());

        if(ImGui::Begin("Viewport")){
            // ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            glm::vec2 viewportPanelSize = glm::vec2{ApplicationInstance::GetWindow().GetWidth(), ApplicationInstance::GetWindow().GetHeight()};

            auto frame_idx = Renderer::GetCurrentFrame();
            // ConsoleLogTrace("Current Frame = {}", frame_idx);
            // auto frame_idx = ApplicationInstance::GetWindow().GetCurrentSwapchain()->AcquireNextImage();
            // auto frame_view = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImageView(frame_idx);
            // if(ImGui::Begin("Image")){
            //     ImGui::Image(g_DescriptorSets[frame_idx], ImVec2{1, 1}, ImVec2{0, 1}, ImVec2{1, 0});
            //     ImGui::End();
            // }
            ImGui::End();
        }

        if(ImGui::Begin("Settings")){
            // static bool dockspace_open = true;
            // ImGui::ShowDemoWindow(&dockspace_open);
            ImGui::End();
        }


        if(ImGui::Begin("Scene Heirarchy")){
            // ImGui::Image(0, ImVec2(ApplicationInstance::GetWindow().GetWidth(), ApplicationInstance::GetWindow().GetHeight()), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
        }

        if(ImGui::Begin("Content Browser")){
            ImGui::End();
        }


        if(ImGui::Begin("Properties Panel")){

            //! @note THERE IS AN ERROR. Where if the imgui docking window is outside of the window
            //! @note Imgui will just have a window that appears until when you exit the application and the UI is not docked outside the window
            ui::DrawPanelComponent<MeshComponent>("Sphere", [&](){
                ui::DrawVec3UI("pos 1", sphere_data.Position);
                ui::DrawVec3UI("scale 1", sphere_data.Scale);
                ui::DrawVec3UI("rotate 1", sphere_data.Rotation);
                ui::LoadFileWithUI("Load Mesh 1", sphere_data.mesh_file);
                // ConsoleLogTrace("mesh_file = {}", sphere_data.mesh_file);
                
                if(sphere_data.mesh_file != ""){
                    std::filesystem::path relative_path = std::filesystem::relative(sphere_data.mesh_file, "./");
                    ConsoleLogTrace("Filepath = {}", sphere_data.mesh_file);
                    ConsoleLogTrace("in branch 1 mesh_file = {}", relative_path.string());
                    // m_Sphere->SetComponent<MeshComponent>({"3d_models\\tutorial\\Ball OBJ.obj"});
                    m_Sphere->SetComponent<MeshComponent>({relative_path.string()});
                    //! TODO: Empty String again 
                    sphere_data.mesh_file = "";
                }
            });

            ui::DrawPanelComponent<MeshComponent>("Some Mesh", [&](){
                ui::DrawVec3UI("Position 2", some_mesh_data.Position);
                ui::DrawVec3UI("Scale 2", some_mesh_data.Scale);
                ui::DrawVec3UI("Rotation 2", some_mesh_data.Rotation);
                ui::DrawFloatUI("Mouse Sensitivity", sensitivity);
                ui::LoadFileWithUI("Load Mesh 2", some_mesh_data.mesh_file);
                // ImGui::Button("")

                // ConsoleLogTrace("mesh_file = {}", some_mesh_data.mesh_file);

                if(some_mesh_data.mesh_file != ""){
                    std::filesystem::path relative_path = std::filesystem::relative(some_mesh_data.mesh_file, "./");
                    ConsoleLogTrace("in branch 2 mesh_file = {}", relative_path.filename().string());
                    m_SomeMesh->SetComponent<MeshComponent>({some_mesh_data.mesh_file});
                    some_mesh_data.mesh_file = "";
                }
            });

            ui::DrawPanelComponent<PerspectiveCamera>("Camera", [&](){
                ui::DrawVec3UI("Position", camera_data.Position);
                ui::DrawVec3UI("Front", camera_data.Front);
            });

            for(auto[key, value] : m_SceneObjectLookup){
                ui::DrawPanelComponent<MeshComponent>(fmt::format("Object {}", key), [&](){
                    auto transform = *value->GetComponent<Transform>();
                    ui::DrawVec3UI(fmt::format("Object {} Pos", key), transform.Position);
                    ui::DrawVec3UI(fmt::format("Object {} Scale", key), transform.Scale);

                    // g_MeshFilepath
                    if(g_MeshFilepath != ""){
                        std::filesystem::path relative_path = std::filesystem::relative(g_MeshFilepath, "./");
                        // ConsoleLogTrace("in branch 2 mesh_file = {}", relative_path.filename().string());
                        value->SetComponent<MeshComponent>({g_MeshFilepath});
                        g_MeshFilepath = "";
                    }

                    value->SetComponent<Transform>({
                        .Position = transform.Position,
                        .Scale = transform.Scale
                    });
                });
            }

            ImGui::End();
        }
    }

    //! TODO: Separate between OnUpdate tasks and OnRender-like tasks
    void LevelScene::OnUpdate() {
        auto camera_transform = *m_Camera->GetComponent<engine3d::Transform>();

        auto camera_comp = *m_Camera->GetComponent<engine3d::PerspectiveCamera>();
        // const engine3d::PerspectiveCamera* perspective_camera = m_Camera->GetComponent<engine3d::PerspectiveCamera>();

        float deltaTime = SyncUpdate::DeltaTime();

        
        if (InputPoll::IsKeyPressed(ENGINE_KEY_LEFT_CONTROL))
        {
            if(InputPoll::IsKeyPressed(ENGINE_KEY_F12))
                ApplicationInstance::GetWindow().Close();
        }

        if (InputPoll::IsKeyPressed(ENGINE_KEY_W)){
            camera_comp.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (InputPoll::IsKeyPressed(ENGINE_KEY_S)){
            camera_comp.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (InputPoll::IsKeyPressed(ENGINE_KEY_A)){
            camera_comp.ProcessKeyboard(LEFT, deltaTime);
        }
        if (InputPoll::IsKeyPressed(ENGINE_KEY_D)){
            camera_comp.ProcessKeyboard(RIGHT, deltaTime);
        }
        if(InputPoll::IsKeyPressed(ENGINE_KEY_LEFT_SHIFT)){
            camera_comp.ProcessKeyboard(UP, deltaTime);
        }
        if(InputPoll::IsKeyPressed(ENGINE_KEY_SPACE)){
            camera_comp.ProcessKeyboard(DOWN, deltaTime);
        }
        
        // **Animation** ---------------------------------------------------

        for(int i = 7; i < 1000; i++)
        {
            auto value = m_SceneObjectLookup[i];
            auto transform = *value->GetComponent<Transform>();

            transform.Position.x += 4 * deltaTime;
            transform.Position.y = sin(transform.Position.x/2) * (200/transform.Position.x);
            transform.Position.z = cos(transform.Position.x/2) * (200/transform.Position.x);
            
            if(transform.Position.x > 721)
            {
                transform.Position.x = 1;
            }

            value->SetComponent(transform);        
        }

        if(!InputPoll::IsKeyPressed(ENGINE_KEY_P))
        {
            auto body = *m_SceneObjectLookup[0]->GetComponent<Transform>();
            auto joint1 = *m_SceneObjectLookup[1]->GetComponent<Transform>();
            auto joint2 = *m_SceneObjectLookup[2]->GetComponent<Transform>();
            auto joint3 = *m_SceneObjectLookup[3]->GetComponent<Transform>();
            auto joint4 = *m_SceneObjectLookup[4]->GetComponent<Transform>();

            auto target = m_SceneObjectLookup[5];
            auto pull = m_SceneObjectLookup[6];

            std::vector<glm::vec3> jointPositions = 
            {
                body.Position,
                joint1.Position,
                joint2.Position,
                joint3.Position,
                joint4.Position
            };

            glm::vec3 targetPos = target->GetComponent<Transform>()->Position;
            glm::vec3 pullPos = pull->GetComponent<Transform>()->Position;
            solveFABRIK(jointPositions, lengths, targetPos, pullPos);
        
            body.Position = jointPositions[0];
            joint1.Position = jointPositions[1];
            joint2.Position = jointPositions[2];
            joint3.Position = jointPositions[3];
            joint4.Position = jointPositions[4];

            m_SceneObjectLookup[0]->SetComponent(body);
            m_SceneObjectLookup[1]->SetComponent(joint1);
            m_SceneObjectLookup[2]->SetComponent(joint2);
            m_SceneObjectLookup[3]->SetComponent(joint3);
            m_SceneObjectLookup[4]->SetComponent(joint4);
        }

        //! @note Press right key and drag the mouse to rotate around
        if (InputPoll::IsMousePressed(ENGINE_MOUSE_BUTTON_RIGHT)) {
            glm::vec2 cursor_pos = InputPoll::GetMousePosition();
            
            //! @note On right click make sure change starts as 0
            if(!on_click_check)
            {
                last_cursor_pos = cursor_pos;
                on_click_check = true;
            }



            //! @note offset is now delta_x and delta_y
            //! @note the difference between mouse old and new positions
            glm::vec2 offset = cursor_pos - last_cursor_pos;

            glm:glm::vec2 velocity = offset * (deltaTime * 100);

            camera_comp.ProcessMouseMovement(velocity.x * ROTATION_DIRECTION_X, 0.0f);

            camera_comp.ProcessMouseMovement(0.0f,velocity.y * ROTATION_DIRECTION_Y);

            last_cursor_pos = cursor_pos;
        } else {
            on_click_check = false;
        }

        


        // OJoy Stick Controls

        // camera_data.Front = m_Camera->GetComponent<PerspectiveCamera>()->Front;
        // camera_data.Position = m_Camera->GetComponent<PerspectiveCamera>()->Position;
        // camera_data.Front = m_Camera->GetComponent<PerspectiveCamera>()->Front;
        // camera_comp.Position = camera_data.Position;
        // camera_comp.Front = camera_data.Front;
        
        // g = x
        // 

        
        //! @note
        camera_comp.MovementSpeed = sensitivity;
        // camera_comp.Position = camera_data.Position;
        // camera_comp.Front = camera_data.Front;
        camera_comp.UpdateProjView();

        // sphere_data.Position.y += (1.0f);
        // sphere_data.Position.x = 

        m_Camera->SetComponent<engine3d::PerspectiveCamera>(camera_comp);
        m_Camera->SetComponent<engine3d::Transform>(camera_transform);

        sphere_data.Position.x = glm::sin(sphere_data.Position.x);
        
        m_Sphere->SetComponent<Transform>({
            .Position = sphere_data.Position,
            .Rotation = sphere_data.Rotation,
            .Scale = sphere_data.Scale,
        });

        m_SomeMesh->SetComponent<Transform>({
            .Position = some_mesh_data.Position,
            .Rotation = some_mesh_data.Rotation,
            .Scale = some_mesh_data.Scale,
        });

        // Renderer::SetCamera(m_Camera);

        //! @note Solved the camera-jittering problem, by making each draw call be a submission call that those objects get called to render at the end of frame
        //! @note Rather then having them be rendered immediately as we update the properties relative to those objects
        // Renderer::RenderWithCamera(m_Sphere, m_Camera);
        // Renderer::SubmitSceneObject(m_Sphere);
        // Renderer::SubmitSceneObject(m_SomeMesh);
        // Renderer::RenderWithCamera(m_Sphere, m_Camera);
        // Renderer::RenderWithCamera(m_SomeMesh, m_Camera);

        // for(auto obj : m_MoreObjects){
        //     Renderer::RenderWithCamera(obj, m_Camera);
        // }

    }

    void LevelScene::PhysicsUpdate(){}

    void LevelScene::OnSceneRender(){
        // ConsoleLogFatal("OnSceneRender Called!");
        Renderer::RenderWithCamera(m_Sphere, m_Camera);
        Renderer::RenderWithCamera(m_SomeMesh, m_Camera);

        for(auto obj : m_MoreObjects){
            Renderer::RenderWithCamera(obj, m_Camera);
        }
    }

    void LevelScene::solveFABRIK(std::vector<glm::vec3>& jointPositions, std::vector<float>& lengths, const glm::vec3& target, const glm::vec3& pull, int maxIterations, float tolerance) 
    { 
        if(jointPositions.empty()) return;

        glm::vec3 root = jointPositions[0]; // Setup the root for bawkward pass 
        int numJoints = jointPositions.size();

        float maxReach = 0.0f;
        for (float len : lengths) maxReach += len;

        float distanceToTarget = glm::length(target - root);

        // Faster iteration if the target is further than the sum
        // of all joint lengths.
        if (distanceToTarget > maxReach) {
            glm::vec3 direction = glm::normalize(target - jointPositions[0]); // Always stretch from current body position
            for (int i = 1; i < numJoints; ++i) {
                jointPositions[i] = jointPositions[i - 1] + direction * lengths[i - 1];
            }
            return;
        }
        // Backward/Forward pass between root and target
        for(int iterator = 0; iterator < maxIterations; iterator++)
        {
            jointPositions[numJoints -1] = target;
            //Backwards pass first

            for(int i = numJoints - 2; i >= 0; --i)
            {
                glm::vec3 direction = glm::normalize(jointPositions[i] - jointPositions[i + 1]);
                jointPositions[i] = jointPositions[i + 1] + direction * lengths[i];
            }

            jointPositions[0] = root;
            for (int i = 1; i < numJoints; ++i) 
            {
                glm::vec3 direction = glm::normalize(jointPositions[i] - jointPositions[i - 1]);
                glm::vec3 newPosition = jointPositions[i - 1] + direction * lengths[i - 1];
                jointPositions[i] = glm::mix(newPosition, pull, PULL_WEIGHT);
            }

            // Close enough to target
            if (glm::length(jointPositions[numJoints - 1] - target) < tolerance) break;
        }
    }
};


/*


RenderPass Attachments API

RenderPass rp = RenderPass();

// Or this should be defined at render pass initialization
rp.SetAttachments({
    {COLOR_FORMAT, sampleCount, NO_LOAD_STORE_STENCIL, NO_IMAGE_DEFINED}
});
*/