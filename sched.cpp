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

/*
 * Simple error debuging code

#include <chrono>
#include <thread>

using namespace std::this_thread;
using namespace std::chrono;

sleep_for(seconds(1));

 */

class SPN : public Scheduler{
    private:
        // service_time이 가장 짧은 Job을 찾아내어 반환하고, job_queue_에서 해당 Job을 제거
        Job search_shortest_job() {
            std::queue<Job> copy_job = job_queue_;
            std::queue<Job> tmp;

            int min_time = 99999;
            Job shortest_job;

            // service_time이 가장 짧은 Job을 서칭
            while (!copy_job.empty()){
                // job_queue_를 복사한 큐에서 가장 첫 요소에 접근하고, 해당 요소 제거
                Job job_value = copy_job.front();
                copy_job.pop();

                // 해당 요소의 service_time이 최솟값인지를 확인하고, 최솟값이라면 저장
                if (job_value.service_time < min_time && job_value.arrival_time <= current_time_) {
                    min_time = job_value.service_time;
                    shortest_job = job_value;
                }
            }

            // service_time이 가장 짧은 Job을 job_queue_에서 제거
            while (!job_queue_.empty()) {
                // service_time이 가장 짧은 Job이 아니라면 요소 추가
                if (job_queue_.front().name != shortest_job.name) {
                    tmp.push(job_queue_.front());
                }
                job_queue_.pop();
            }

            // job_queue_ 업데이트
            job_queue_ = tmp;

            // service_time이 가장 짧은 job을 반환
            return shortest_job;
        }

    public:
        // 자식 클래스 생성자
        SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SPN";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = search_shortest_job();
            }

            // 현재 작업이 모두 완료되면
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (job_queue_.empty()) return -1;

                // 새로운 작업 할당
                current_job_ = search_shortest_job();
                // context switch 타임 추가
                current_time_ += switch_time_;
            }

            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;

            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

class RR : public Scheduler{
    private:
        int time_slice_;
        int left_slice_;
        std::queue<Job> waiting_queue;

    public:
        // 자식 클래스 생성자
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }else {
                // 현재 작업이 모두 완료되면
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);

                    // 남은 작업이 없으면 종료
                    if (job_queue_.empty() && waiting_queue.empty()) return -1;

                    // 새로운 작업 할당
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();

                    // context switch 타임 추가
                    current_time_ += switch_time_;
                    // time quantum 제한 시간 초기화
                    this->left_slice_ = this->time_slice_;
                
                // time_quantum에 의해 다음 작업으로 넘어가야 하는 경우
                }else if (left_slice_ == 0) {
                    // ready 큐(변수 명 waiting_queue)에 현재 작업을 올림
                    waiting_queue.push(current_job_);
                    int prev_job_name = current_job_.name;

                    // 현재 작업을 ready 큐에서 획득하고 ready 큐에서 삭제
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();

                    // context switch가 일어난 경우에 대해서만 context switch 타임 추가
                    if (prev_job_name != current_job_.name) current_time_ += switch_time_;
                    // time quantum 제한 시간 초기화
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
            // time quantum 제한까지 남은 시간 --
            this->left_slice_--;

            // 들어온 작업들 ready 큐에 추가
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

            // job_queue 원소가 1개밖에 없는 경우에는 함수 실행하지 않도록 함
            if (job_queue_.size() == 1) {
                return;
            }

            // 현재까지 진행한 작업의 reain_time이 0이 아니라면 해당 작업으로 값 초기화
            if (current_job_.remain_time != 0) {
                shortest_job = current_job_;
                min_time = current_job_.remain_time;
                min_job_name = current_job_.name;
            }

            // remain_time이 가장 짧은 작업 탐색
            while (!job_queue_.empty()){
                // job_queue_의 가장 첫 요소에 접근하고, 해당 요소 복제 및 job_queue_에서 제거
                Job job_value = job_queue_.front();
                job_queue_.pop();
                tmp.push(job_value);

                // 해당 요소가 remain_time이 더 짧고, 도착한 작업이라면
                if (job_value.remain_time <= min_time && job_value.arrival_time <= current_time_) {
                    // remain_time이 동일한데 더 늦게 온 작업인 경우 무시
                    if (job_value.remain_time == min_time) {
                        if (job_value.name > min_job_name) {
                            continue;
                        }
                    }
                    // remain_time이 더 짧은 작업이라면 업데이트
                    shortest_job = job_value;
                    min_time = job_value.remain_time;
                    min_job_name = job_value.name;
                }
            }

            // job_queue의 가장 앞 부분에 remain_time이 가장 짧은 Job 위치
            job_queue_.push(shortest_job);

            // 이를 제외한 나머지 요소들 job_queue에 순서대로 다시 추가
            while (!tmp.empty()){
                Job job_value = tmp.front();
                tmp.pop();

                if (job_value.name != shortest_job.name) {
                    job_queue_.push(job_value);
                }
            }
        }

    public:
        // 자식 클래스 생성자
        SRT(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SRT";
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                move_job_to_front();
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }else {
                // 현재 작업이 모두 완료되면
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);

                    // 남은 작업이 없으면 종료
                    if (job_queue_.empty()) return -1;

                    // remain_time이 가장 짧은 Job을 찾고 새로운 작업 할당
                    move_job_to_front();
                    current_job_ = job_queue_.front();
                    job_queue_.pop();

                    // context switch 타임 추가
                    current_time_ += switch_time_;
                }else {
                    int prev_job_name = current_job_.name;
                    // 이전 작업을 job_queue_에 추가
                    job_queue_.push(current_job_);

                    // remain_time이 가장 짧은 Job을 찾고 새로운 작업 할당
                    move_job_to_front();
                    current_job_ = job_queue_.front();
                    job_queue_.pop();

                    // context switch 타임 추가
                    if (prev_job_name != current_job_.name) current_time_ += switch_time_;
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

            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

class HRRN : public Scheduler{
    private:
        // response_ratio가 가장 높은 Job을 찾아내어 반환하고, job_queue_에서 해당 Job을 제거
        Job search_high_response_ratio_job() {
            std::queue<Job> copy_job = job_queue_;
            std::queue<Job> tmp;

            double high_response_ratio = 0;
            Job high_job;

            // response_ratio가 가장 높은 Job을 서칭
            while (!copy_job.empty()){
                // job_queue_를 복사한 큐에서 가장 첫 요소에 접근하고, 해당 요소 제거
                Job job_value = copy_job.front();
                copy_job.pop();

                if (job_value.arrival_time <= current_time_) {
                    double job_response_ratio = 1 + double(current_time_ - job_value.arrival_time) / job_value.service_time;
                    
                    // 해당 요소의 response_ratio가 최댓값인지를 확인하고, 최댓값이라면 저장
                    if (job_response_ratio > high_response_ratio) {
                        high_response_ratio = job_response_ratio;
                        high_job = job_value;
                    }
                }
            }

            // response_ratio이 가장 높은 Job을 job_queue_에서 제거
            while (!job_queue_.empty()) {
                // sresponse_ratio이 가장 높은 Job이 아니라면 요소 추가
                if (job_queue_.front().name != high_job.name) {
                    tmp.push(job_queue_.front());
                }
                job_queue_.pop();
            }

            // job_queue_ 업데이트
            job_queue_ = tmp;

            // response_ratio가 가장 높은 작업 반환
            return high_job;
        }
    public:
        HRRN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "HRRN";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = search_high_response_ratio_job();
            }

            // 현재 작업이 모두 완료되면
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (job_queue_.empty()) return -1;

                // 새로운 작업 할당
                current_job_ = search_high_response_ratio_job();
                // context switch 타임 추가
                current_time_ += switch_time_;
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

            // 스케줄링할 작업명 반환
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

        // ready_queue에서 각 우선 순위 큐들이 모두 비어있는지 확인하는 함수
        bool is_ready_queue_empty() {
            bool flag = true;
            int index = 0;
            // 큐의 개수가 4로 고정되어 있으므로 0순위 큐부터 3순위 큐까지 탐색
            while (index < 4) {
                // 큐가 비어있지 않다면 flag를 false로 바꾸고 반복 종료
                if (!ready_queue[index].empty()) {
                    flag = false;
                    break;
                }

                index++;
            }

            // ready_queue가 완전히 비어있는지 반환
            return flag;
        }

        // 우선 순위 큐를 탐색하면서 실행할 작업 업데이트하는 함수
        void search_next_job() {
            // ready_queue에서 우선 순위가 가장 높은 작업 할당
            this->current_queue = 0;
            // 우선 순위가 가장 높은 곳부터 탐색
            while(this->current_queue < 4) {
                // 해당 큐가 비어있다면 다음 순위의 큐로 넘어감
                if (ready_queue[this->current_queue].empty()) {
                    this->current_queue++;
                // 해당 큐가 비어있지 않다면 가장 앞에 있는 원소를 획득
                }else {
                    current_job_ = ready_queue[this->current_queue].front();
                    ready_queue[this->current_queue].pop();
                    break;
                }
            }
            // 모든 우선 순위 큐가 비어 있음을 의미하므로 최상위 할당
            if (this->current_queue == 4) {
                this->current_queue = 0;
            }
        }

    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if(is_2i){
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }

            // time_quantum이 2^i라면 1, 2, 4, 8의 값을 갖도록 하고, 아니라면 1로 고정
            if (is_2i) {
                quantum = {1, 2, 4, 8};
            }else {
                quantum = {1, 1, 1, 1};
            }

            // ready_queue의 크기를 4로 사전 초기화 (빈 영역 확인을 위함)
            this->ready_queue.resize(4);
            // time_quantum 제한 시간 초기화
            this->left_slice_ = quantum[current_queue];
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }else {
                // 현재 작업이 모두 완료되면
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);

                    // 남은 작업이 없으면 종료
                    if (job_queue_.empty() && is_ready_queue_empty()) return -1;

                    // 새로운 작업 할당 (우선 순위 큐 탐색)
                    search_next_job();

                    // context switch 타임 추가
                    current_time_ += switch_time_;
                    // time_quantum 제한 시간을 우선 순위에 맞게 조정
                    this->left_slice_ = quantum[this->current_queue];
                // time_quantum에 의해 종료되는 경우
                }else if (this->left_slice_ == 0) {
                    int prev_job_name = current_job_.name;

                    // 이미 가장 낮은 우선 순위에 있는 작업이라면
                    if (this->current_queue == 3) {
                        // 현재 우선 순위 큐에 진행한 작업 추가
                        ready_queue[this->current_queue].push(current_job_);
                        // 새로운 작업 할당 (우선 순위 큐 탐색)
                        search_next_job();
                    // 가장 낮은 우선 순위에 있는 작업이 아닌 경우
                    }else {
                        int prev_job_queue = this->current_queue;

                        // 이전 작업의 우선 순위가 최하위거나, 이전 작업의 우선 순위가 최상위인데 다른 큐들이 모두 비어있는 경우 큐 이동하지 않도록 함
                        if (prev_job_queue == 3 || is_ready_queue_empty()) {
                            ready_queue[prev_job_queue].push(current_job_);
                        // 위의 예외에 해당하지 않는 경우 우선 순위가 하나 낮은 큐로 이전 작업 이동
                        }else {
                            ready_queue[prev_job_queue + 1].push(current_job_);
                        }

                        // 새로운 작업 할당 (우선 순위 큐 탐색)
                        search_next_job();
                    }

                    // context switch가 일어난 경우에 대해서만 context switch 타임 추가
                    if (prev_job_name != current_job_.name) current_time_ += switch_time_;

                    // time_quantum 제한 시간을 우선 순위에 맞게 조정
                    this->left_slice_ = quantum[this->current_queue];
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
            // time quantum 제한까지 남은 시간 --
            this->left_slice_--;

            // 들어온 작업들 ready 큐에 추가
            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                Job add_job = job_queue_.front();
                ready_queue[0].push(add_job);
                job_queue_.pop();
            }

            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};
