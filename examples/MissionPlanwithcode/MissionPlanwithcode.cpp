#include <iostream> 
#include <thread> 
#include <chrono>
#include <mavsdk/mavsdk.h> 
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
#include <future>
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
    
    mission_item.latitude_deg = 47.3980836; // range: -90 to +90 1
    mission_item.longitude_deg = 8.5468915; // range: -180 to +180
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    mission_item.latitude_deg = 47.3994970; // range: -90 to +90 2
    mission_item.longitude_deg = 8.5476075; // range: -180 to +180
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    mission_item.latitude_deg = 47.4000072; // range: -90 to +90 3
    mission_item.longitude_deg = 8.5467174; // range: -180 to +180
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    mission_item.latitude_deg = 47.3992419; // range: -90 to +90 4 
    mission_item.longitude_deg = 8.5426357; // range: -180 to +180
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    mission_item.latitude_deg = 47.3969187; // range: -90 to +90 5
    mission_item.longitude_deg = 8.5411682; // range: -180 to +180 
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    mission_item.latitude_deg = 47.3961478; // range: -90 to +90 6
    mission_item.longitude_deg = 8.5452338; // range: -180 to +180
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    mission_item.latitude_deg = 47.3979826; // range: -90 to +90 7 
    mission_item.longitude_deg = 8.5469018; // range: -180 to +180
    mission_item.relative_altitude_m = 10.0f; // takeoff altitude
    mission_item.speed_m_s = 5000.0f;
    mission_item.is_fly_through = false; // stop on the waypoint
    mission_items.push_back(mission_item);

    int mission_size = mission_items.size();
    std::cout << "mission size : " << mission_size << std::endl;

    //@@ Fly Mission2
    auto prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result_01 = prom->get_future();
    Mission::MissionPlan mission_plan;
    mission_plan.mission_items = mission_items;
    auto mission = std::make_shared<Mission>(system);
    mission->upload_mission_async(mission_plan,[prom](Mission::Result result){prom->set_value(result);
    });
    const Mission::Result result = future_result_01.get();
    if(result != Mission::Result::Success) {return 1;}  //Mission upload failed
    

    //@@ ARM

    auto action = std::make_shared<Action>(system);
    std::cout << "arming...."<<std::endl;
    const Action::Result arm_result = action->arm();
    if (arm_result != Action::Result::Success){
        std::cout << "Arming failed:" <<arm_result << std::endl;
        
    }

    //@@ Fly Mission Progress
 
    std::cout << "Starting mission." <<std::endl;
    auto start_prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result_02 = start_prom->get_future();
    mission->start_mission_async([start_prom](Mission::Result result){
        start_prom->set_value(result);
        std::cout << "Started mission." << std::endl;
    });
    const Mission::Result start_result = future_result_02.get();
    if(start_result != Mission::Result::Success) { return -1; }
    while(!mission->is_mission_finished().second){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    // std::cout << "taking off..."<<std::endl;
    // const Action::Result takeoff_result = action->takeoff();
    // if (takeoff_result != Action::Result::Success){
    //     std::cout << "taking off failed..." <<arm_result << std::endl;
        
    // }

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
