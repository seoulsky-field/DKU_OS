/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32213947
*	    Student name : Kyungmin Jeon
*/

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include "sched.h"

#include <chrono>
#include <thread>

class SPN : public Scheduler{
    private:
        Job search_shortest_job() {
            std::queue<Job> copy_job = job_queue_;
            std::queue<Job> tmp;

            int min_time = 99999;
            Job shortest_job;

            // Find the shortest service_time Job
            while (!copy_job.empty()){
                Job job_value = copy_job.front();
                copy_job.pop();

                if (job_value.service_time < min_time && job_value.arrival_time <= current_time_) {
                    min_time = job_value.service_time;
                    shortest_job = job_value;
                }
            }

            // Remove the shortest service time Job in the job_queue_
            while (!job_queue_.empty()) {
                if (job_queue_.front().name != shortest_job.name) {
                    tmp.push(job_queue_.front());
                }
                job_queue_.pop();
            }

            job_queue_ = tmp;

            return shortest_job;
        }

    public:
        SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SPN";
        }

        int run() override {
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = search_shortest_job();
            }

            if (current_job_.remain_time == 0) {
                current_job_.completion_time = current_time_;
                end_jobs_.push_back(current_job_);

                if (job_queue_.empty()) return -1;

                current_job_ = search_shortest_job();

                current_time_ += switch_time_;
            }

            if (current_job_.service_time == current_job_.remain_time) {
                current_job_.first_run_time = current_time_;
            }

            current_time_++;

            current_job_.remain_time--;

            return current_job_.name;
        }
};

class RR : public Scheduler{
    private:
        int time_slice_;
        int left_slice_;
        std::queue<Job> waiting_queue;
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
    public:
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }

        int run() override {
            /*
            * 구현 (아래 코드도 수정 및 삭제 가능)
            */
            return -1;
        }
                
};

class SRT : public Scheduler{
    private:
        void move_job_to_front() {
            std::queue<Job> tmp;
            int min_time = 99999;
            int min_job_name = 99999;
            Job shortest_job;

            if (job_queue_.size() == 1) {
                return;
            }

            if (current_job_.remain_time != 0) {
                shortest_job = current_job_;
                min_time = current_job_.remain_time;
                min_job_name = current_job_.name;
            }

            // Find the shortest remain_time Job
            while (!job_queue_.empty()){
                Job job_value = job_queue_.front();
                job_queue_.pop();
                tmp.push(job_value);

                if (job_value.remain_time <= min_time && job_value.arrival_time <= current_time_) {
                    if (job_value.remain_time == min_time) {
                        if (job_value.name > min_job_name) {
                            continue;
                        }
                    }
                    shortest_job = job_value;
                    min_time = job_value.remain_time;
                    min_job_name = job_value.name;
                }
            }

            job_queue_.push(shortest_job);

            while (!tmp.empty()){
                Job job_value = tmp.front();
                tmp.pop();

                if (job_value.name != shortest_job.name) {
                    job_queue_.push(job_value);
                }
            }
        }

    public:
        SRT(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SRT";
        }

        int run() override {
            if (current_job_.name == 0 && !job_queue_.empty()) {
                move_job_to_front();
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }else {
                if (current_job_.remain_time == 0) {
                    current_job_.completion_time = current_time_;
                    end_jobs_.push_back(current_job_);

                    if (job_queue_.empty()) return -1;

                    move_job_to_front();
                    current_job_ = job_queue_.front();
                    job_queue_.pop();

                    current_time_ += switch_time_;
                }else {
                    int prev_job_name = current_job_.name;
                    job_queue_.push(current_job_);

                    move_job_to_front();
                    current_job_ = job_queue_.front();
                    job_queue_.pop();

                    if (prev_job_name != current_job_.name) current_time_ += switch_time_;
                }
            }

            if (current_job_.service_time == current_job_.remain_time) {
                current_job_.first_run_time = current_time_;
            }

            current_time_++;
            current_job_.remain_time--;

            return current_job_.name;
        }
};

class HRRN : public Scheduler{
    private:
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */    
    public:
        HRRN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "HRRN";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */            
        }

        int run() override {
            /*
            구현 
            */
            return -1;
        }
};

// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler{
    private:
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */    
    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if(is_2i){
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }

            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
        }

        int run() override {
            /*
            * 구현 
            */
            return -1;
        }
};
