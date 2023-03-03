#pragma once

#include <stack>
#include <algorithm>
#include <vector>

using namespace std;

struct Point {
    int x, y;
};

class ConvexHull {
public:
    
    ConvexHull(vector<Point>& points) {
        this->points = points;
        n = points.size();
        iniP0();
    }
    void iniP0(){
        // 找到y坐标最小的点p0，作为起点
        int p0_index = 0;
        for (int i = 1; i < n; i++) {
            if (points[i].y < points[p0_index].y) {
                p0_index = i;
            } else if (points[i].y == points[p0_index].y && points[i].x < points[p0_index].x) {
                p0_index = i;
            }
        }
        swap(points[0], points[p0_index]);
        this->p0 = points[0];
    }
    void run() {
        
        // 将点按极角从小到大排序
        sortPoints();

        // 压入栈中，判断栈顶三个点是否构成左转
        stack<Point> s;
        s.push(points[0]);
        s.push(points[1]);
        s.push(points[2]);
        for (int i = 3; i < n; i++) {
            while (s.size() >= 2) {
                Point p2 = s.top();
                s.pop();
                Point p1 = s.top();
                if ((p1.x - p2.x) * (points[i].y - p2.y) - (p1.y - p2.y) * (points[i].x - p2.x) > 0) {
                    s.push(p2);
                    break;
                }
            }
            s.push(points[i]);
        }

        // 将凸包顶点存入result
        while (!s.empty()) {
            result.push_back(s.top());
            s.pop();
        }
        reverse(result.begin(), result.end());
    }

    vector<Point> getResult() {
        return result;
    }

private:
    vector<Point> points;
    vector<Point> result;
    Point p0;
    int n;
    //p0p1与p0p2比较角度，det表示p1和p2的向量叉积，大于零，它们构成左转。如果向量叉积等于0，则根据距离的平方大小来比较两个点，距离平方越小的点排在前面。
    bool compare(Point p1, Point p2) {
        int det = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        if (det == 0) {
            return (p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y) <
                   (p2.x - p0.x) * (p2.x - p0.x) + (p2.y - p0.y) * (p2.y - p0.y);
        } else {
            return det > 0;
        }
    }
    void sortPoints() {
        if(sameNum(points)){
            return;
        }
        quickSort(points, 1, points.size()-1);
    }
    void quickSort(vector<Point>& nums, int start, int end){
        if(start >= end ) return;
        int pivot_index = rand() % (end - start + 1) + start;
        swap(nums[start],nums[pivot_index]);
        int left = start;
        int right = end;
        while(left < right){
            while(left < right && !compare(nums[start], nums[right])) right--;
            while(left < right && compare(nums[start], nums[left])) left++;
            swap(nums[left], nums[right]);
        }
        swap(nums[start], nums[left]);
        quickSort(nums, start, left - 1);
        quickSort(nums, left +1, end);

    }
    bool sameNum(vector<Point>& nums){
        int minIndex = 0, maxIndex=0;
        for(int i = 0; i < nums.size(); i++){
            if(compare(nums[i],nums[maxIndex])) maxIndex = i;
            if(!compare(nums[i],nums[minIndex])) minIndex = i;
        }
        if (maxIndex == minIndex)
            return true; 
        else
            return false;
    }
};
/*
int main() {
    vector<Point> points = {{1, 2}, {2, 1}, {3, 4}, {4, 5}, {5, 3}};
    ConvexHull ch(points);
    ch.run();
    vector<Point> result = ch.getResult();
    for (Point p : result) {
        cout << "(" << p.x << ", " << p.y << ")\n";
    }
    return 0;
}
*/