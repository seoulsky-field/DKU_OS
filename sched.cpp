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
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }else {
                if (current_job_.remain_time == 0) {
                    current_job_.completion_time = current_time_;
                    end_jobs_.push_back(current_job_);

                    if (job_queue_.empty() && waiting_queue.empty()) return -1;

                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();

                    current_time_ += switch_time_;
                    this->left_slice_ = this->time_slice_;
                }else if (left_slice_ == 0) {
                    waiting_queue.push(current_job_);
                    int prev_job_name = current_job_.name;

                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();

                    if (prev_job_name != current_job_.name) current_time_ += switch_time_;

                    this->left_slice_ = this->time_slice_;
                }
            }

            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time){
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;

            this->left_slice_--;

            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                Job add_job = job_queue_.front();
                waiting_queue.push(add_job);
                job_queue_.pop();
            }

            // 스케줄링할 작업명 반환
            return current_job_.name;
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
        Job search_high_response_ratio_job() {
            std::queue<Job> copy_job = job_queue_;
            std::queue<Job> tmp;

            double high_response_ratio = 0;
            Job high_job;

            // Find the shortest service_time Job
            while (!copy_job.empty()){
                Job job_value = copy_job.front();
                copy_job.pop();

                if (job_value.arrival_time <= current_time_) {
                    double job_response_ratio = 1 + double(current_time_ - job_value.arrival_time) / job_value.service_time;
                    
                    if (job_response_ratio > high_response_ratio) {
                        high_response_ratio = job_response_ratio;
                        high_job = job_value;
                    }
                }
            }

            // Remove the shortest service time Job in the job_queue_
            while (!job_queue_.empty()) {
                if (job_queue_.front().name != high_job.name) {
                    tmp.push(job_queue_.front());
                }
                job_queue_.pop();
            }

            job_queue_ = tmp;

            return high_job;
        }
    public:
        HRRN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "HRRN";
        }

        int run() override {
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = search_high_response_ratio_job();
            }

            if (current_job_.remain_time == 0) {
                current_job_.completion_time = current_time_;
                end_jobs_.push_back(current_job_);

                if (job_queue_.empty()) return -1;

                current_job_ = search_high_response_ratio_job();

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

// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler{
    private:
        std::vector<std::queue<Job>> ready_queue;
        std::vector<int> quantum;

        int left_slice_;
        int current_queue = 0;

        bool is_ready_queue_empty() {
            bool flag = true;
            int index = 0;
            while (index < 4) {
                if (!ready_queue[index].empty()) {
                    flag = false;
                    break;
                }

                index++;
            }

            return flag;
        }

        // void search_next_job() {
        //     int prev_job_queue = this->current_queue;
        //     Job next_job_;
        //     this->current_queue = 0;

        //     while(this->current_queue < 4) {
        //         if (ready_queue[this->current_queue].empty()) {
        //             this->current_queue++;
        //         }else {
        //             next_job_ = ready_queue[this->current_queue].front();
        //             ready_queue[this->current_queue].pop();
        //             break;
        //         }
        //     }

        //     if (this->current_queue == 4) {
        //         this->current_queue = 0;
        //     }else {
        //         if (prev_job_queue == 3) {
        //             ready_queue[prev_job_queue].push(current_job_);
        //         }else {
        //             ready_queue[prev_job_queue + 1].push(current_job_);
        //         }
                
        //         current_job_ = next_job_;
        //     }

        //     this->left_slice_ = quantum[this->current_queue];
        // }

    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if(is_2i){
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }

            if (is_2i) {
                quantum = {1, 2, 4, 8};
            }else {
                quantum = {1, 1, 1, 1};
            }
            this->ready_queue.resize(4);
            this->left_slice_ = quantum[current_queue];
        }

        int run() override {
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }else {
                if (current_job_.remain_time == 0) {
                    current_job_.completion_time = current_time_;
                    end_jobs_.push_back(current_job_);

                    if (job_queue_.empty() && is_ready_queue_empty()) return -1;

                    this->current_queue = 0;
                    while(this->current_queue < 4) {
                        if (ready_queue[this->current_queue].empty()) {
                            this->current_queue++;
                        }else {
                            current_job_ = ready_queue[this->current_queue].front();
                            ready_queue[this->current_queue].pop();
                            break;
                        }
                    }

                    current_time_ += switch_time_;
                    this->left_slice_ = quantum[this->current_queue];
                }else if (this->left_slice_ == 0) {
                    int prev_job_name = current_job_.name;

                    if (this->current_queue == 3) {
                        ready_queue[this->current_queue].push(current_job_);

                        this->current_queue = 0;
                        while(this->current_queue < 4) {
                            if (ready_queue[this->current_queue].empty()) {
                                this->current_queue++;
                            }else {
                                current_job_ = ready_queue[this->current_queue].front();
                                ready_queue[this->current_queue].pop();
                                break;
                            }
                        }
                    }else {
                        int prev_job_queue = this->current_queue;
                        Job next_job_;
                        this->current_queue = 0;

                        if (prev_job_queue == 3 || is_ready_queue_empty()) {
                            ready_queue[prev_job_queue].push(current_job_);
                        }else {
                            ready_queue[prev_job_queue + 1].push(current_job_);
                        }

                        while(this->current_queue < 4) {
                            if (ready_queue[this->current_queue].empty()) {
                                this->current_queue++;
                            }else {
                                next_job_ = ready_queue[this->current_queue].front();
                                ready_queue[this->current_queue].pop();
                                break;
                            }
                        }

                        if (this->current_queue == 4) {
                            this->current_queue = 0;
                        }else {
                            current_job_ = next_job_;
                        }
                    }

                    if (prev_job_name != current_job_.name) current_time_ += switch_time_;

                    this->left_slice_ = quantum[this->current_queue];
                }
            }

            if (current_job_.service_time == current_job_.remain_time) {
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;

            this->left_slice_--;

            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                Job add_job = job_queue_.front();
                ready_queue[0].push(add_job);
                job_queue_.pop();
            }

            // using namespace std::this_thread;
            // using namespace std::chrono;

            // std::cout << current_job_.name << "\n";
            // std::cout << "current queue: " << this->current_queue << "\n";

            // sleep_for(seconds(1));

            return current_job_.name;
        }
};
