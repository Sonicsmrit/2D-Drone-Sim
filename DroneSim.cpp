#include <iostream>
#include <raylib.h>
#include <string>
#include <raymath.h>

using namespace std;
int hight = 800;
int width = 1000;
int groundY = 800;

class Drone{

    Texture2D drone_texture;
    bool isloaded = false;
    Vector2 DronePosition;
    Vector2 velocity = {0, 0};
    float omega = 0.0f;
    float theta = 0.0f;
    float throttle = 0.0f;

    public:

    //loading drone tecture.
    Drone(const string &droneFilePath){

        Image droneImg = LoadImage(droneFilePath.c_str());
        ImageResize(&droneImg, 100, 100);

        drone_texture = LoadTextureFromImage(droneImg);
        UnloadImage(droneImg);

        if(drone_texture.id != 0){
            isloaded = true;
        }
        else{
            cout<<"Failed to load drone image!"<<'\n';
        }

        //position x, y
        DronePosition= {(float)(width/2), (float) hight - (drone_texture.height/2)};
        // DronePosition = {100, 100};

    }

    //unload texture
    void Unload(){
        if(isloaded){
            UnloadTexture(drone_texture);
            isloaded = false;
        }
    }
    
    Texture2D Gettexture(){
        return drone_texture;
    }

    Vector2 &GetPosition(){
        return DronePosition;
    }

    float &GetTheta(){
        return theta;
    }

    Vector2 &GetVel(){
        return velocity;
    }

    float &Getthrottle(){
        return throttle;
    }

    float &GetOmega(){
        return omega;
    }

};


class Physics{

    Drone &drone;
    float thrust = 0.0f;
    float thrust_left_engine = 0.0f;
    float thrust_right_engine = 0.0f;
    Vector2 wind = {0,0}; //wind moves the drone in air
    float target =  GetRandomValue(-100, 100);
    bool onGround;
    float Windtimer = 10.0f;

    public:
    Physics(Drone &targetDrone) : drone(targetDrone) {}

    float Getthrust(){
        return thrust;
    }
    float GetLeftEngine(){
        return thrust_left_engine;
    }
    float GetRightEngine(){
        return thrust_right_engine;
    }
    Vector2 GetWind(){
        return wind;
    }

    //Stops drone from going out of view
    void Walls(){
        Texture2D texture = drone.Gettexture();
        Vector2 &DronePosition = drone.GetPosition();
        Vector2 &vel = drone.GetVel();
        float &theta = drone.GetTheta();
        float &omega = drone.GetOmega();
        float dt = GetFrameTime();
        float friction = 5.0;


        int texWidth = texture.width;
        int texHeight = texture.height;

        if(DronePosition.y >= groundY - (texHeight/2)){
            DronePosition.y = groundY - (texHeight/2);
            vel.y = 0;
            vel.x *= 1 - friction * dt;
            theta = 0;
            omega = 0;

            onGround = true;

        }
        else{
            onGround = false;
        }



    }

    void Update(){
        float g = 500.0f; //[px/s^2] gravity
        Vector2 &DronePosition = drone.GetPosition();
        Vector2 &vel = drone.GetVel();
        float &theta = drone.GetTheta();
        Vector2 accelerator = {0, g};
        float dt = GetFrameTime();
        float maxThrust = 900.0;
        float rate = 1.0;
        float &throttle = drone.Getthrottle();
        float steering = 0.0f;
        float alpha = 0.0f;
        float &omega = drone.GetOmega();
        float k = 0.02; //
        float c = 4.0; //damping cofficent
        float Dragcoff = 0.5f;

        Windtimer -= dt;

        if(Windtimer <= 0){
            target = GetRandomValue(-100, 100);
            Windtimer = 20;
        }

        //For the wind
        float RateofChange = 0.5f;

        if(IsKeyDown(KEY_W)){
            throttle += rate * dt;

            throttle = Clamp(throttle, 0, 1);
    
        }

        if(IsKeyDown(KEY_S)){
            throttle -= rate * dt;
            throttle = Clamp(throttle, 0, 1);
        }

        if(IsKeyDown(KEY_A)){
            steering -= 0.2;
        }

        if(IsKeyDown(KEY_D)){
            steering += 0.2;
        }

        thrust_left_engine = (throttle + steering) * maxThrust / 2;

        thrust_left_engine = Clamp(thrust_left_engine, 0, maxThrust/2);

        thrust_right_engine = (throttle - steering) * maxThrust/2;

        thrust_right_engine = Clamp(thrust_right_engine, 0, maxThrust/2);

        alpha = k * (thrust_left_engine - thrust_right_engine);

        omega += alpha * dt;

        theta += omega * dt;


        thrust = thrust_left_engine + thrust_right_engine;

        if(!onGround){
            wind.x += (target - wind.x) * RateofChange * dt;

        }
        else{
            wind.x = 0;

        }

        wind.y = 0;

        accelerator.x += thrust * sin(theta);
        accelerator.y -= thrust * cos(theta);
        
        //drag
        accelerator.x -= Dragcoff * (vel.x - wind.x);
        accelerator.y -= Dragcoff * (vel.y - wind.y);

        vel.x += accelerator.x * dt; 
        vel.y += accelerator.y * dt;

        DronePosition.y += vel.y * dt; 

        DronePosition.x += vel.x * dt;

        //damping

        omega *= 1- c * dt;

    }

};

class Render{
    public:
    //drone
    void DrawDrone(Drone &drone){

        Texture2D texture = drone.Gettexture();
        Vector2 pos = drone.GetPosition();
        float theta = drone.GetTheta();
        

        Rectangle source = {0, 0, (float)texture.width, (float)texture.height}; // source: the whole image, starting from its top-left corner

        Rectangle destination = {pos.x, pos.y, (float)texture.width, (float)texture.height}; //where it lands on screen. x/y is the CENTER (pos), width/height is the on-screen size

        Vector2 origin = {texture.width/2.0f, texture.height/2.0f};

        DrawTexturePro(texture, source, destination, origin, theta * RAD2DEG, WHITE);

    }

    void DrawHUD(Physics &phy, Drone &drone){
        DrawText("THRUST:", 10, 10, 35, RED);
        DrawText(TextFormat("Total: %.1f", phy.Getthrust()), 10, 50, 20, RED);
        DrawText(TextFormat("Left Engine: %.1f", phy.GetLeftEngine()), 10, 90, 20, RED);
        DrawText(TextFormat("Right Engine: %.1f", phy.GetRightEngine()), 10, 130, 20, RED);
        DrawText(TextFormat("Throttle: %.1f", drone.Getthrottle()),10,170, 25, RED);
        DrawText(TextFormat("Total Speed: %.1f",Vector2Length(drone.GetVel())), 10, 210, 20, RED);  //sqrt(vel.x² + vel.y²)
        DrawText(TextFormat("Wind Direction %.0f", (float)phy.GetWind().x), 10, 250, 20, RED);
        DrawText(TextFormat("Altitude: %.0f", groundY -(drone.GetPosition().y + drone.Gettexture().height/2)), 810, 10, 25, RED);

    }

    //background patterns
    void DrawPattern(Camera2D &cam){
        int Size = 40;
        Vector2 world_left_edge = GetScreenToWorld2D({0,0}, cam);
        Vector2 world_right_botton = GetScreenToWorld2D({(float)width, (float)hight}, cam);

        //grid lines

        int startX = (int)floorf(world_left_edge.x / Size) * Size;
        int startY = (int)floorf(world_left_edge.y / Size) * Size;

        for (int x = startX; x <= world_right_botton.x; x += Size) {
            DrawLine(x, (int)world_left_edge.y, x, (int)world_right_botton.y, Fade(BLACK, 0.25f));
        }

        for (int y = startY; y <= world_right_botton.y; y += Size) {
            DrawLine((int)world_left_edge.x, y, (int)world_right_botton.x, y, Fade(BLACK, 0.25f));
        }

        if(groundY < world_right_botton.y){
            DrawRectangle((int)world_left_edge.x, groundY, (int)(world_right_botton.x - world_left_edge.x) + 1, (int)(world_right_botton.y - groundY) + 1, DARKGRAY);
        }
    }



};


int main(){

    InitWindow(width, hight, "Windy day drone");

    Drone drone("../drone.png");

    Physics phy(drone);

    Render rd;

    SetTargetFPS(60);

    Camera2D cam{{(float)(width/2), (float)(hight/2)}, drone.GetPosition(), 0.0f, 1.0f};

    while(!WindowShouldClose()){

        phy.Update();
        phy.Walls();
        cam.target = drone.GetPosition();

        BeginDrawing();

        ClearBackground(WHITE);
        BeginMode2D(cam);
        rd.DrawPattern(cam);
        rd.DrawDrone(drone);

        EndMode2D();

        rd.DrawHUD(phy, drone);

        EndDrawing();
    }

    drone.Unload();

    CloseWindow();



    return 0;
}