#include <cstdlib>

#include "Networking/Player.h"
#include "Tools/octetStream.h"
#include "Tools/int.h"



using namespace std;
int main(int argc, char** argv)
{
    if (argc != 2){
        cerr<<"Call using\n\t";
        cerr<<"./test_float.x ID ";
        cerr<<"\t\t ID          = Number of machines"<<endl;
        exit(1);
    }

    int player_no = atoi(argv[1]);
    int portnum_base = 6000;

    string filename = "Test/HOSTS.example";
    Names player_name = Names(player_no, portnum_base, filename);
    ThreadPlayer P(player_name, "");

    string msg = "The msg is from P" + to_string(player_no);
    octetStream os_to_send(msg);
    P.send_all(os_to_send);

    octetStreams os(3);
    P.receive_all_no_stats(os);
    for(int i = 0; i < P.num_players(); i++){
        if(i!=P.my_num()){
            cout<<os[i].str()<<endl;
        }
    }

}
