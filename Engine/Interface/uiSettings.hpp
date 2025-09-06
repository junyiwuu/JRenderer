


namespace UI{    

enum class UserCam {ArcballCamera, FirstPersonCamera};


struct UISettings{
    UserCam userCam = UserCam::ArcballCamera;

    //default value
    float baseColor[3] = {1.f, 1.f, 1.f};
    float roughness = 0.5f;
    float metallic = 0.f;
    
    bool inputAlbedoPath = false;
    char albedoTexPath[256];

    bool inputRoughnessPath = false;
    char roughnessTexPath[256];

    bool inputMetallicPath = false;
    char metallicTexPath[256];

    bool inputNormalPath = false;
    char normalTexPath[256];

};







};