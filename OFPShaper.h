#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "Session.h"
#include "SwitchSession.h"
class OFPShaper
{
public:
    OFPShaper(size_t shape_rate):shape_rate_(shape_rate),is_acked_(true)
    {

    }

    void setDest(std::function<void(Message)> dest)
    {

    }

    void setSource(std::function<Message(void)> source)
    {
        source_ = source;
    }


    bool startShaper()
    {
        if(source_ != NULL && dest_ !=NULL )
        {
            shaper_ = std::make_shared<std::thread>(std::bind( &OFPShaper::process_, this));
            return true;
        }
        else
        {
            return false;
        }
    }

    //called by other thread
    void OnAck(Message message)
    {
        cv_.notify_one();
    }

private:


    void process_()
    {
        while(true)
        {
            std::unique_lock<std::mutex> lock(mt_);
            while(!is_acked_)
            {
                cv_.wait(lock);
            }


        }
    };

private:
    size_t shape_rate_;
    std::function<Message(void)> source_;
    std::function<void(Message)> dest_;
    std::shared_ptr<std::thread> shaper_;
    bool is_acked_;
    std::mutex mt_;
    std::condition_variable cv_;
};
