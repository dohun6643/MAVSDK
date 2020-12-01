#include <iostream> 
#include <thread> 
#include <chrono>
#include <mavsdk/mavsdk.h> 
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
using namespace mavsdk;

int main(int argc, char** argv)
{
    Mavsdk mavsdk;
    ConnectionResult connection_result;
    bool discovered_system = false;
    connection_result = mavsdk.add_any_connection(argv[1]);

    if(connection_result != ConnectionResult::Success){ return 1; }
    mavsdk.subscribe_on_new_system([&mavsdk,&discovered_system](){
        const auto system = mavsdk.systems().at(0);
        if(system->is_connected()) { discovered_system = true; }
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    if(!discovered_system){  return 1;}
    
    const auto system = mavsdk.systems().at(0);
    system->register_component_discovered_callback(
        [](ComponentType component_type)->void{std::cout << unsigned(component_type) ;}
    );          //Register a callback components(camera,gimbal) etc are found



auto telemetry = std::make_shared<Telemetry>(system);
// We want to listen to the altitude of the drone at 1 Hz.
const Telemetry::Result set_rate_result = telemetry->set_rate_position(1.0);
if(set_rate_result != Telemetry::Result::Success){return 1;}
telemetry->subscribe_position([](Telemetry::Position position){
    std::cout << "Altitude: " << position.relative_altitude_m << " m" << std::endl;
});
while(telemetry->health_all_ok()!=true){
    std::cout << "Vehicle is getting ready to arm"<< std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}                               //Check if vehicle is ready to arm

//@@ Fly Mission
std::vector<Mission::MissionItem> mission_items;
Mission::MissionItem mission_item;
mission_item.latitude_deg = 47.398170327054473; // range: -90 to +90
mission_item.longitude_deg = 8.5456490218639658; // range: -180 to +180
mission_item.relative_altitude_m = 10.0f; // takeoff altitude
mission_item.speed_m_s = 5.0f;
mission_item.is_fly_through = false; // stop on the waypoint
mission_items.push_back(mission_item);
int mission_size = mission_items.size();
std::cout << mission_size << std::endl;
//

auto action = std::make_shared<Action>(system);
std::cout << "arming...."<<std::endl;
const Action::Result arm_result = action->arm();
if (arm_result != Action::Result::Success){
    std::cout << "Arming failed:" <<arm_result << std::endl;
    
}

std::cout << "taking off..."<<std::endl;
const Action::Result takeoff_result = action->takeoff();
if (takeoff_result != Action::Result::Success){
    std::cout << "taking off failed..." <<arm_result << std::endl;
    
}

std::this_thread::sleep_for(std::chrono::seconds(10));
std::cout << "Landing..."<<std::endl;
const Action::Result land_result = action->land();
if (land_result != Action::Result::Success){ return 1;}
while (telemetry->in_air()){
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
    std::cout << "Landed!" << std::endl;
    //Relying on auto-disarming but let's keep watching the telemetry for a bit longer
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Finished..." << std::endl;
return 0;
}
