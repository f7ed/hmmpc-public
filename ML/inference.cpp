#include "NeuralNet/NeuralNetConfig.h"
#include "NeuralNet/NeuralNetwork.h"
#include "NeuralNet/secondary.h"
#include "Protocols/PhaseConfig.h"
#include "NeuralNet/globals.h"

using namespace hmmpc;
using namespace std;
int partyNum;

// partyNum = argv[1]
// filename = argv[2] ->nplayers
// threshold = argv[3]
// network = argv[4]
// dataset = argv[5]
// test_data_size = argv[6]
// mini_batch = argv[6]
// trueOffline = argv[7]
// offline_arg = argv[8]
// CORES = argv[9]
int main(int argc, char** argv)
{
    int cores = atoi(argv[9]);
    Eigen::setNbThreads(cores);
    // cout<<"#threads: "<<Eigen::nbThreads()<<endl;

    // Build Communication Channels
    partyNum = atoi(argv[1]);
    int portnum_base = 6000;
    string filename = argv[2];
    Names player_name = Names(partyNum, portnum_base, filename);
    ThreadPlayer P(player_name, "");
    int threshold = atoi(argv[3]);

    PhaseConfig phase;
    phase.init(P.num_players(), threshold, &P);

    // Select Network
    NeuralNetConfig * config = new NeuralNetConfig(NUM_ITERATIONS);
    string network, dataset;
    // network = "SecureML";
    network = argv[4];
    dataset = argv[5];
    loadData(network, dataset, atoi(argv[6]));
    
    selectNetwork(network, dataset, config);
    NeuralNetwork *net = new NeuralNetwork(config);
    preload_netwok(true, network, net);

    if(atoi(argv[7])){// Set true offline.
        phase.setTrueOffline();
        phase.start_offline();
        string offline_arg = argv[8];
        phase.generate_random(offline_arg);
        phase.end_offline();
    }

    phase.start_online();
    // train(net);
    test(net);
    phase.end_online();

    // phase.print_simple();

    if(!atoi(argv[7])){// True offline
        // The randomness are generated on demand.
        // It prints the communication costs of each party in the terminal. 
        phase.print_offline_communication();
        phase.print_online_communication();
    }else{
        // The randomness are generated in a seperate pre-processing phase.
        // The communication size printed here is the average of the communicaiton bytes sent by all parties.
        phase.print_communication_oneline();
    }
    
}