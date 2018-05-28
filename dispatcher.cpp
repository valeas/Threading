#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <vector>

//
// supporting tools and software
//
// Validate and test your json commands
// https://jsonlint.com/

// RapidJSON : lots and lots of examples to help you use it properly
// https://github.com/Tencent/rapidjson
//

// std::function
// std::bind
// std::placeholders
// std::map
// std::make_pair

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

bool g_done = false;


//
// TEST COMMANDS
//
auto help_command = R"(
 {
  "command":"help",
  "payload": {
    "usage":"Enter json command in 'command':'<command>','payload': { // json payload of arguments }",
  }
 }
)";

auto exit_command = R"(
 {
  "command":"exit",
  "payload": {
     "reason":"Exiting program on user request."
  }
 }
)";

//this class simulates production controlled by the controller. It is not connected to it, but serves as an example of what I was thinking of
class Machine {
public:
    //this one should not exits in actual use, but is nice for testing
    Machine(){
       commands.push_back("start");
       commands.push_back("status");
       commands.push_back("stop");
       commands.push_back("setSpeed");
    }
    //the actual contructor
    Machine(vector<string> commands_for_machine){
        commands=commands_for_machine;
    }
    ~Machine(){
    }

    vector<string> getCommands(){
        return commands;
    }
    bool start(){
        if (running){
            return false;
        }
        running=true;
        return true;
    }

    bool stop(){
        if (!running){
            return false;
        }
        running=false;
        return true;
    }

    bool isRunning(){
        return running;
    }

    bool setSpeed(int speed){
        processSpeed=speed;
        return true;
    }

private:
    vector<string> commands;
    bool running=false;
    int processSpeed=0;

};


class Controller {
public:
    bool help(rapidjson::Value &payload)
    {
        cout<< "List of available commands:"<<endl;
        cout<< "  exit , params: string reason(optional)"<<endl;
        cout<< "  stop , params: string reason"<<endl;
        cout<< "  start , params: string reason"<<endl;
        cout<< "  help"<<endl;
        cout<< "  status"<<endl;
        cout<< "  setSpeed , params: string reason, int value"<<endl;
        return true;
    }

    bool exit(rapidjson::Value &payload)
    {
        //cout << "Controller::exit: command: \n";
        cout<< "exiting dispatcher"<<endl;
        if (payload.HasMember("reason") && payload["reason"].IsString()){
            cout<<"Reason: "<<payload["reason"].GetString()<<endl;  }
        g_done=true;
        return true;
    }

    bool start(rapidjson::Value &payload)
    {
        cout<< "Starting machine:"<<endl;
        if (running){
            cout<< "Error: the machine is already running"<<endl;
            return false;
        }
        if (payload.HasMember("reason") && payload["reason"].IsString()){
           cout<<"Reason: "<<payload["reason"].GetString()<<endl;  }
        else {return false;}

        cout<<"LOGGING "<<running << endl;
        running=true;
        cout<< "LOG NOW"<<running<<endl;
        return true;
    }

    bool stop(rapidjson::Value &payload)
    {
        cout<< "Stopping machine:"<<endl;
        if (!running){
            cout<< "Error: the machine is already stopped"<<endl;
            return false;
        }
        if (payload.HasMember("reason") && payload["reason"].IsString()){
            cout<<"Reason: "<<payload["reason"].GetString()<<endl;  }
        else {return false;}
        running=false;
        return true;
    }

    bool isRunning(rapidjson::Value &payload)
    {
        cout<< "Checking machine status:"<<endl;
        if (running){
            cout<<"The machine is running"<<endl;
        } else {
            cout<<"The machine is stopped"<<endl;
        }
        return true;
    }

    bool setSpeed(rapidjson::Value &payload)
    {
        cout<< "Setting new speed:"<<endl;
        if (payload.HasMember("value") && payload["value"].IsInt()){
            cout << "Changing speed from "<< speed << "to " <<payload["value"].GetInt() <<endl;
            speed= payload["value"].GetInt();
        }
        else {return false;}
        if (payload.HasMember("reason") && payload["reason"].IsString()){
          cout<<"Reason: "<<payload["reason"].GetString()<<endl;  }
        else {return false;}
        return true;
    }
private:
    bool running =false;
    int speed = 0;

};

// gimme ... this is actually tricky
// Bonus Question: why did I type cast this?  //so we can map a funtion from a class instance without having to write it all the time
typedef std::function<bool(rapidjson::Value &)> CommandHandler;

class CommandDispatcher {
public:
    CommandDispatcher()
    {
        //no commands constructor;
        available_commands.push_back("help");
        available_commands.push_back("exit");
    }

    CommandDispatcher(vector<string> commands)
    {
        available_commands=commands;
        available_commands.push_back("help");
        available_commands.push_back("exit");
    }

    // dtor - need impl
    virtual ~CommandDispatcher()
    {
        // question why is it virtual? Is it needed in this case?

        //a destructor is not really needed as the class is never destroyed(if it is, the program stops working)
        //also, we can still use it if we call a pointer to this if we want to delete a derived class
    }

    bool addCommandToList(string cmd){
        for (int i=0; i<available_commands.size(); i++){
            if (cmd==available_commands[i]){
                return false; //command in list
            }
        }
        available_commands.push_back(cmd);
        return true;
    }

    bool addCommandHandler(string command, CommandHandler handler)
    {
        cout << "CommandDispatcher: addCommandHandler: " << command << std::endl;
        for(int i=0; i<available_commands.size(); i++){
            if (command==available_commands[i]){
                command_handlers_.insert(pair<string, CommandHandler>(available_commands[i], handler));
                return true;
            }
        }
        return false;
    }

    bool dispatchCommand(std::string command_json)
    {

        cout << "COMMAND: " << command_json << endl;
        Document document;
        Value v;
        document.Parse(command_json.c_str());
        if (document.HasMember("command") && document["command"].IsString()){
            string cmd=document["command"].GetString();
            //printf("got command = %s\n", cmd); this one is a no-go
            //cout<<"got command: "<<cmd<<endl;

            //use the found command
            if (command_handlers_.find(cmd)!= command_handlers_.end()){
                CommandHandler c = command_handlers_.find(cmd)->second;
                return c(document["payload"]);
            }
        }
        cout<< "Command not recognized"<<endl;
        return false;
    }

private:
    // gimme ...
    map<string, CommandHandler> command_handlers_;

    vector<string> available_commands; //this allows a dispather to use multiple controllers(and therefore simplyfies adding multiple machines)

    // another gimme ...
    // Question: why delete these?
    //Answer: We do not need these as dispatcher should be unique
    //(to prevent conflicts when equipment receives command from multiple sources and keep the state consistent within the program instance)

    // delete unused constructors
    CommandDispatcher (const CommandDispatcher&) = delete;
    CommandDispatcher& operator= (const CommandDispatcher&) = delete;

};

int main()
{
    std::cout << "COMMAND DISPATCHER: STARTED" << std::endl;
    Controller controller;                 // controller class of functions to "dispatch" from Command Dispatcher

    // add command handlers in Controller class to CommandDispatcher using addCommandHandler
    Machine worker;
    CommandDispatcher command_dispatcher(worker.getCommands());
    command_dispatcher.addCommandHandler("exit", std::bind(&Controller::exit, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("start", std::bind(&Controller::start, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("stop", std::bind(&Controller::stop, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("status", std::bind(&Controller::isRunning, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("help", std::bind(&Controller::help, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("setSpeed", std::bind(&Controller::setSpeed, &controller, std::placeholders::_1));

    // gimme ...
    // command line interface
    string command;
    while( ! g_done ) {

        //uncomment for console copy-paste example commands
       /* cout << "COMMANDS: {\"command\":\"exit\", \"payload\":{\"reason\":\"User requested exit.\"}}\n";
        cout << "          {\"command\":\"start\", \"payload\":{\"reason\":\"User requested machine start.\"}}\n";
        cout << "          {\"command\":\"stop\", \"payload\":{\"reason\":\"User requested machine stop.\"}}\n";
        cout << "          {\"command\":\"status\", \"payload\":{}}\n";
        cout << "          {\"command\":\"help\", \"payload\":{}}\n";
        cout << "          {\"command\":\"setSpeed\", \"payload\":{\"reason\":\"User requested new speed setting.\" , \"value\":100}}\n";*/

        cout << "\tenter command : ";
        getline(cin, command);
        command_dispatcher.dispatchCommand(command);
    }

    std::cout << "COMMAND DISPATCHER: ENDED" << std::endl;
    return 0;
}
