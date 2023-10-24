#include "PhaseConfig.h"
#include "Protocols/RandomShare.h"
namespace hmmpc
{

void PhaseConfig::init(int n, int t, ThreadPlayer *_P, int _Pking)
{
    ShareBase::n_players = n;
    ShareBase::threshold = t;
    ShareBase::P = _P;
    ShareBase::Pking = _Pking;
    ShareBase::Phase = this; // Bind the phase pointer.

    // Suppose this the the random seed agreed among the parties.
    const char key[SEED_SIZE]="ThisIs7heSeed.";
    ShareBase::PRNG_agreed.SetSeed((const octet*)key);
    // cout<<ShareBase::PRNG_agreed.get_uint()<<endl;

    ShareBase::init_vandermondes();
    ShareBase::init_reconstruction_vectors();

    ShareBase::init_bits_coeff();

    ShareBase::send_buffers.reset(n);
    ShareBase::receive_buffers.reset(n);

    ShareBase::send_buffers_PRG.reset(t+1);
    ShareBase::receive_buffers_PRG.reset(n);
}

void PhaseConfig::set_input_file(string fn)
{
    ShareBase::set_input_file(fn);
}

void PhaseConfig::close_input_file()
{
    ShareBase::close_input_file();
}

void PhaseConfig::start_offline()
{
    isOffline = true;
    auto &P = ShareBase::P;
    P->comm_stats.clear();
    P->timer.reset();
    P->sent = 0;
    P->timer.start();
}

void PhaseConfig::end_offline()
{
    isOffline = false;
    auto &P = ShareBase::P;
    P->timer.stop();
    offlineTimer += P->timer.elapsed();
    offlineSent += P->sent;
    offlineComm += P->comm_stats;
}

void PhaseConfig::clear_offline_status()
{
    offlineTimer = 0;
    offlineSent = 0;
    offlineComm.clear();
}

void PhaseConfig::print_offline_communication()
{
    auto &P = ShareBase::P;
    cout << "---OFFLINE----"<<endl;
    cout << "Time = "<<offlineTimer<<" seconds"<<endl;
    cout << "Data sent = "<< offlineSent/1e6 <<" MB in ~"<<offlineComm.total_rounds()<<" rounds"<<endl;
    octetStream o;
    o.store(offlineSent);
    P->send_all_no_stats(o);
    octetStreams os;
    P->receive_all_no_stats(os);
    size_t global = offlineSent;
    for(int i = 0; i < P->num_players(); i++)
        if(i != P->my_num())
            global += os[i].get_int(8);
    cout << "Global data sent = "<<global/1e6<<" MB (all parties)"<<endl;
    if(!is_true_offline()){
        cout <<"Preprocessed Randomness:"<<endl;
        cout <<"#Random = "<<cntRandom<<endl;
        cout <<"#DoubleR = "<<cntReducedRandom<<endl;
        cout <<"#RandomBit = "<<cntRandomBit<<endl;
        cout <<"#RandomUn = "<<cntUnboundedMultRandom<<" "<<constUnboundedSize<<endl;
        cout <<"#RandomTrunc = "<<cntTruncatedRandom<<endl;
        cout <<"#RandomReTrunc = "<<cntReducedTruncatedRandom<<endl;
        // cout <<"#RandomTrunc(ML) = "<<cntTruncatedRandomInML<<endl;
        // cout <<"#RandomReTrunc(ML) = "<<cntReducedTruncatedInMLRandom<<endl;
        // cout <<"#RandomReTrunc(diff) = "<<cntRTRandomWithDifferentPrecision<<endl;
    }
    
    cout << "-------------"<<endl;
}

void PhaseConfig::start_online()
{
    isOnline = true;
    auto &P = ShareBase::P;
    P->comm_stats.clear();
    P->timer.reset();
    P->sent = 0;
    P->timer.start();
}



void PhaseConfig::end_online()
{
    isOnline = false;
    auto &P = ShareBase::P;
    P->timer.stop();
    onlineTimer += P->timer.elapsed();
    onlineComm += P->comm_stats;
    onlineSent += P->sent;
}

void PhaseConfig::clear_online_status()
{
    onlineTimer = 0;
    onlineComm.clear();
    onlineSent = 0;
}

void PhaseConfig::print_online_communication()
{
    auto &P = ShareBase::P;
    cout << "---ONLINE---"<<endl;
    cout << "Time = "<<onlineTimer<<" seconds"<<endl;
    cout << "Data sent = "<< onlineSent/1e6 <<" MB in ~"<<onlineComm.total_rounds()<<" rounds"<<endl;
    octetStream o;
    o.store(onlineSent);
    P->send_all_no_stats(o);
    octetStreams os;
    P->receive_all_no_stats(os);
    size_t global = onlineSent;
    for(int i = 0; i < P->num_players(); i++)
        if(i != P->my_num())
            global += os[i].get_int(8);
    cout << "Global data sent = "<<global/1e6<<" MB (all parties)"<<endl;
    cout << "-------------"<<endl;
}
size_t PhaseConfig::get_global_communication(string phase)
{
    size_t nSent;
    if(phase.compare("online")==0){
        nSent = onlineSent;
    }else if (phase.compare("offline")==0){
        nSent = offlineSent;
    }else if(phase.compare("all")==0){
        nSent = offlineSent+onlineSent;
    }else{
        assert(false && "get_global_communication");
    }

    auto &P = ShareBase::P;
    octetStream o;
    o.store(nSent);
    P->send_all_no_stats(o);
    octetStreams os;
    P->receive_all_no_stats(os);
    size_t global = nSent;
    for(int i = 0; i < P->num_players(); i++)
        if(i != P->my_num())
            global += os[i].get_int(8);
    return global;
}

void PhaseConfig::print_communication_oneline()
{
    auto &P = ShareBase::P;
    // communication per party
    cout<<onlineTimer<<" "<<get_global_communication("online")/1e6/P->num_players()<<" ";
    cout<<offlineTimer<<" "<<get_global_communication("offline")/1e6/P->num_players()<<" ";
    cout<<get_global_communication("all")/1e6/P->num_players()<<endl;
}


void PhaseConfig::print_online_comm_online()
{
    cout<<onlineTimer<<" "<<get_global_communication("online")/1e6<<endl;
}

void PhaseConfig::switch_to_offline()
{
    assert(isOnline);
    end_online();
    start_offline();
}

void PhaseConfig::switch_to_online()
{
    assert(isOffline);
    end_offline();
    start_online();
}

void PhaseConfig::generate_random(string filename)
{
    ifstream in(filename);
// #Random = 5232650
// #DoubleR = 7714250
// #RandomBit = 1262090
// #RandomUn = 248160 8
// #RandomTrunc = 0
// #RandomReTrunc = 10350
    for(int i = 0; i < 6; i++){
        int x;
        in>>x;
        if(x){
            switch (i)
            {
            case 0:
                generate_random_sharings(x);
                break;
            case 1:
                generate_reduced_random_sharings(x);
                break;
            case 2:
                generate_random_bits(x);
                break;
            case 3:
                int y;
                in>>y;
                generate_unbounded_mult_random_sharings(x, y);
                break;
            case 4:
                generate_truncated_random_sharings(x);
                break;
            case 5:
                generate_reduced_truncated_sharings(x);
                break;
            default:
                break;
            }
        }
        
    }
}

void PhaseConfig::generate_random_sharings(size_t n)
{
    cntRandom += n;
    // RandomShare::generate_random_sharings(n);
    RandomShare::generate_random_sharings_PRG(n);
}

void PhaseConfig::generate_random_bits(size_t n)
{
    cntRandomBit += n;
    RandomShare::generate_random_bits(n);
}

void PhaseConfig::generate_reduced_random_sharings(size_t n)
{
    cntReducedRandom += n;
    // DoubleRandom::generate_reduced_random_sharings(n);
    DoubleRandom::generate_reduced_random_sharings_PRG(n);
}

void PhaseConfig::generate_truncated_random_sharings(size_t n)
{
    cntTruncatedRandom += n;
    DoubleRandom::generate_truncated_random_sharings(n);
}

void PhaseConfig::generate_truncated_random_sharings(size_t n, size_t precision)
{
    cntTruncatedRandomInML += n;
    DoubleRandom::generate_truncated_random_sharings(n, precision);
}

void PhaseConfig::generate_reduced_truncated_sharings(size_t n)
{
    cntReducedTruncatedRandom += n;
    DoubleRandom::generate_reduced_truncated_random_sharings(n);
}

// Combine the reduced truncation with the opration *learning_rate/batch_size.
void PhaseConfig::generate_reduced_truncated_sharings(size_t n, size_t logLearningRate, size_t logMiniBatch)
{
    cntReducedTruncatedInMLRandom += n;
    DoubleRandom::generate_reduced_truncated_random_sharings(DoubleRandom::queueReducedTruncatedInML, n, FIXED_PRECISION+logLearningRate+logMiniBatch);
}

void PhaseConfig::generate_reduced_truncated_sharings(size_t n, size_t precision)
{
    cntRTRandomWithDifferentPrecision += n;
    DoubleRandom::generate_reduced_truncated_random_sharings(DoubleRandom::queueReducedTruncatedWithPrecisionRandom ,n, precision);
}

void PhaseConfig::generate_reduced_truncated_sharings(size_t n, vector<size_t> &precision, size_t num_repetition)
{
    cntRTRandomWithDifferentPrecision += n * num_repetition;
    DoubleRandom::generate_reduced_truncated_random_sharings(DoubleRandom::queueReducedTruncatedWithPrecisionRandom ,n, precision, num_repetition);
}

void PhaseConfig::generate_unbounded_mult_random_sharings(size_t xSize, size_t ySize)
{
    cntUnboundedMultRandom += xSize;
    if(constUnboundedSize==0){constUnboundedSize=ySize;}
    else if(ySize!=constUnboundedSize)assert(false && "Unbounded ySize is inconsistent");
    DoubleRandom::generate_unbounded_random_sharings(xSize, ySize);
}

void PhaseConfig::generate_unbounded_mult_random_sharings(size_t num)
{
    // cntUnboundedMultRandom += num;
    DoubleRandom::generate_unbounded_random_sharings(num);
}

}