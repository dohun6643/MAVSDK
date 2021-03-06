#include <iostream> 
#include <thread> 
#include <chrono>
#include <mavsdk/mavsdk.h> 
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
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
    return 0;
}