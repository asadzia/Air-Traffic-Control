#include <iostream>
#include <future>
#include <vector>
#include <mutex>
#include <ctime>
#include <chrono>
#include <thread>
#include <atomic>

using namespace std;

#define AIRCRAFTS 3
#define GATES 2
#define RUNWAYS 1

//atomic<bool> landed(false);
//atomic<bool> gate(false);

mutex output;

void init(mutex mtx_g[], mutex mtx_r[], int plane)
{
    //this_thread::sleep_for(chrono::milliseconds(100));
    /* PRINT MESSAGES */
    output.lock();
    cout << "[AC]   Flight " << plane << " requesting landing" << endl;
    output.unlock();

    //land
    srand(time(NULL));
    bool landed = false;
    bool gate = false;
    
    int runawayTime = rand()%(5000-2000 + 1) + 5000;
    try 
    {
         while (landed == false)
        {
            for (int i = 0; i < RUNWAYS; i++)
            {
                if (mtx_r[i].try_lock())
                {
                    /* PRINT MESSAGES */
                    output.lock();
                    cout << "[AC]   Airplane " << plane << " assigned to runway " << i + 1 <<  endl;
                    cout << "[AC]   Airplane " << plane << " has landed" <<  endl;
                    output.unlock();
                    
                    this_thread::sleep_for(chrono::milliseconds(2000));
                    landed = true;
                    mtx_r[i].unlock();
                    break;
                }
            }
        
            if (landed) {break;}
        }
    }
    catch (exception e)
    {
        output.lock();
        cout << "Hi. " << e.what() << " Plane: " << plane <<  endl;
        output.unlock();
    }
   
         
    /* PRINT MESSAGES */
    output.lock();
    cout << "[AC]   Airplane " << plane << " requesting gate" << endl;
    output.unlock();

    //dock to gate
    while (gate == false)
    {
        for (int j = 0; j < GATES; ++j)
        {
            if (mtx_g[j].try_lock())
            {
                /* PRINT MESSAGES */
                output.lock();
                cout << "[AC]   Airplane " << plane << " assigned gate " << j + 1 << endl;
                output.unlock();
                
                this_thread::sleep_for(chrono::milliseconds(runawayTime));
                gate = true;
                mtx_g[j].unlock();
                break;
            }
        }

        if (gate) {break;}
    }

    //heading to hangar
    /* PRINT MESSAGES */
    output.lock();
    cout << "[AC]   Airplane " << plane << " heading to hangar" << endl;
    output.unlock();
}


int main (int argc, char** args)
{
    // Airport Setup
    vector <future<void> > airplanes;

    // Mutex
    mutex mtx_g[GATES];
    mutex mtx_r[RUNWAYS];

    for (int i = 0; i < AIRCRAFTS; i++)
    {
        airplanes.push_back(async(launch::async, &init, mtx_g, mtx_r, i + 1));
    }

    for (auto i = 0; i < AIRCRAFTS; ++i)
     {
        airplanes[i].get();
     }


    return 0;
}
