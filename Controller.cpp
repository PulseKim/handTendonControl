#include "Controller.hpp"
#include <iostream>
#include <dart/dart.hpp>
#include <dart/gui/gui.hpp>
#include <dart/utils/utils.hpp>

using namespace dart::common;
using namespace dart::dynamics;
using namespace dart::math;


const double mag_Kp = 400;
const double mag_Kd = 2 * std::sqrt(mag_Kp);
int count = 5;
const double rad = M_PI /180.0;

Controller::Controller(const SkeletonPtr& finger, std::vector<Tendon*> tendon): mFinger(finger){
    mTendon = tendon;
    jointControlSetter();
}

void Controller::jointControlSetter(){
	int nDofs = mFinger->getNumDofs();

	mForces = Eigen::VectorXd::Zero(nDofs);
    Ycontroller = Eigen::VectorXd::Zero(nDofs);
    mTendonForces = Eigen::VectorXd::Zero(mFinger->getNumJoints());
    mKp = Eigen::MatrixXd::Identity(nDofs, nDofs);
    mKd = Eigen::MatrixXd::Identity(nDofs, nDofs);

    for(std::size_t i = 0; i < 6; ++i){
      mKp(i,i) = mag_Kp;
      mKd(i,i) = mag_Kd;
    }
    for(std::size_t i = 6; i < 21; ++i){
      double mag_new = mag_Kp/2;
      mKp(i,i) = mag_new;
      mKd(i,i) = 2*std::sqrt(mag_new);
    }
    for(std::size_t i = 21; i < nDofs; ++i){
      double mag_new = mag_Kp/6;
      mKp(i,i) = mag_new;
      mKd(i,i) = 2*std::sqrt(mag_new);
    }
  Controller::setTargetPosition(mFinger->getPositions());
}

void Controller::setTargetPosition(const Eigen::VectorXd& pose){
	mTargetPositions = pose;
}

void Controller::clearForces(){
	mForces.setZero();
	mTendonForces.setZero();
    Ycontroller.setZero();
}

void Controller::addPDForces(){
	Eigen::VectorXd q = mFinger->getPositions();
    Eigen::VectorXd dq = mFinger->getVelocities();
    Eigen::VectorXd p = -mKp * (q - mTargetPositions);
    Eigen::VectorXd d = -mKd * dq;
    
    mForces += p + d;
    mFinger->setForces(mForces);
}


void Controller::addSPDForces(){
	Eigen::VectorXd q = mFinger->getPositions();
    Eigen::VectorXd dq = mFinger->getVelocities();
    for(int i = 0; i < q.size(); ++i){
        if(q[i]-mTargetPositions[i] > M_PI) q[i]-=2*M_PI;
        else if(q[i]-mTargetPositions[i] < -M_PI) q[i]+=2*M_PI;
    }

    Eigen::MatrixXd invM = (mFinger->getMassMatrix()
        + mKd * mFinger->getTimeStep()).inverse();
    Eigen::VectorXd p =
    -mKp * (q + dq * mFinger->getTimeStep() - mTargetPositions);
    Eigen::VectorXd d = -mKd * dq;
    Eigen::VectorXd qddot =
    invM * (-mFinger->getCoriolisAndGravityForces()
        + p + d + mFinger->getConstraintForces());
    
    mForces += p + d - mKd * qddot * mFinger->getTimeStep();
    mFinger->setForces(mForces);

    // std::cout << "Current pose and force " << std::endl;
    // std::cout << q[2] /rad << std::endl;
    // std::cout << mTargetPositions[2] / rad << std::endl;
    // std::cout << dq[2] << std::endl;
    // std::cout << mForces[2] << std::endl;
}


void Controller::addSPDTendonForces(){
	Eigen::VectorXd q = mFinger->getPositions();
	Eigen::VectorXd dq = mFinger->getVelocities();

	Eigen::MatrixXd invM = (mFinger->getMassMatrix()
        + mKd * mFinger->getTimeStep()).inverse();
    Eigen::VectorXd p =
    -mKp * (q + dq * mFinger->getTimeStep() - mTargetPositions);
    Eigen::VectorXd d = -mKd * dq;
    Eigen::VectorXd qddot =
    invM * (-mFinger->getCoriolisAndGravityForces()
        + p + d + mFinger->getConstraintForces());
    
    mForces += p + d - mKd * qddot * mFinger->getTimeStep();

    double default_width = 0.5;
    double linearForce;

    double z_angle = (M_PI - q[0])/2;
    linearForce = (mForces[0]) / (default_width / 2 * ((z_angle > 0) - (z_angle < 0))* std::max(std::abs(std::sin(z_angle)), 0.005));
    mTendon[0]->ApplyForceToBody(std::max(linearForce, 0.0));
    mTendon[1]->ApplyForceToBody(std::max(-linearForce, 0.0));
    //if(true) std::cout  << " is "<< linearForce << std::endl;


    for(std::size_t i=2 ; i < mTendon.size(); ++i){
        double current_angle = (M_PI - q[i])/2;
        linearForce = -(mForces[i]) / (default_width / 2 * ((current_angle > 0) - (current_angle < 0))* std::max(std::abs(std::sin(current_angle)), 0.005));      
        mTendon[i]->ApplyForceToBody(linearForce);       

    }
    count--;

}

void Controller::addSPDTendonDirectionForces(){
    Eigen::VectorXd q = mFinger->getPositions();
    Eigen::VectorXd dq = mFinger->getVelocities();

    Eigen::MatrixXd invM = (mFinger->getMassMatrix()
        + mKd * mFinger->getTimeStep()).inverse();
    Eigen::VectorXd p =
    -mKp * (q + dq * mFinger->getTimeStep() - mTargetPositions);
    Eigen::VectorXd d = -mKd * dq;
    Eigen::VectorXd qddot =
    invM * (-mFinger->getCoriolisAndGravityForces()
        + p + d + mFinger->getConstraintForces());

    mForces += p + d - mKd * qddot * mFinger->getTimeStep();

    double default_width = 0.5;
    double linearForce;

    //Ycontroller[0] = mForces[0];
    //Ycontroller[1] = mForces[1];
    // // /Ycontroller[2] = mForces[2];
    // Ycontroller[3] = mForces[3];
    // Ycontroller[4] = mForces[4];
    mFinger->setForces(Ycontroller);

    std::size_t i;
    //count = 1;
    // if(count > 0 ) std::cout <<" is "<< std::endl;

    for(i = 0 ; i < mFinger->getNumJoints()-1; ++i){
        Eigen::Vector3d joint_point = 
        mFinger->getBodyNode(mFinger->getNumJoints()-1-i)->getTransform() * Eigen::Vector3d(0, -1, 0);
        double torque_wanted = mForces[mForces.size()-1-i] - prevTorque(joint_point);
        linearForce = -(torque_wanted/ (default_width / 2)) ;
        std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> temp = 
        mTendon[mFinger->getNumJoints()-2 -i]->ApplyForceToBodyDir(linearForce);
        for(std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>::iterator iter = temp.begin(); iter != temp.end();++iter){
            prevForces.push_back(*iter);
        }
        // if(count > 0 ) 
        // {
        //     std::cout << "Real" << linearForce <<std::endl;
        //         //std::cout << i << " is "<< linearForce << std::endl;
        // }
    }
    count--;
    linearForce = mForces[0] / (default_width / 2);
    mTendon[i]->ApplyForceToBodyDir(std::max(linearForce, 0.0));
    mTendon[i+1]->ApplyForceToBodyDir(std::max(-linearForce, 0.0));
    prevForces.clear();
}

double Controller::prevTorque(const Eigen::Vector3d current_point){
    double torque_sum = 0.0;
    // if(count > 0) std::cout << "came" << std::endl;
    for(int i = 0; i < prevForces.size(); ++i){
        Eigen::Vector3d r_vector = prevForces[i].second - current_point;
        torque_sum += r_vector.cross(prevForces[i].first)[2];
        // if(count > 0 ) 
        // {
        //     std::cout << r_vector.cross(prevForces[i].first)<< std::endl;
        // }
    }
    //if(count > 0) std::cout << torque_sum <<std::endl;
    return torque_sum;
}

Eigen::VectorXd Controller::grabOrOpen(Eigen::VectorXd originalPose, bool isGrabbed){
    Eigen::VectorXd pose = originalPose;
    if(isGrabbed){
        // // for(int i = 6 ; i< 22 ; ++i){
        // //     pose[i] = 30.0* rad;
        // // }
        // // pose[22] = 100.0 * rad;
        // // pose[23] = -40.0 * rad;
        // // pose[24] = 30.0 * rad;
        for(int i = 0 ; i< 4 ; ++i){
            pose[i*4 + 7] = 66.0* rad;
            pose[i*4 + 8] = 45.0* rad;
            pose[i*4 + 9] = 66.0* rad;
        }
        pose[22] = 90.0 * rad;
        pose[23] = -40.0 * rad;
        pose[24] = 30.0 * rad;
        pose[25] = 40.0 * rad;
        pose[26] = 40.0 * rad;
    }
    else{
        for(int i = 0 ; i< 4 ; ++i){
            pose[i*4 + 7] = 0.0* rad;
            pose[i*4 + 8] = 0.0* rad;
            pose[i*4 + 9] = 0.0* rad;
        }
        pose[22] = 50.0 * rad;
        pose[23] = -30.0 * rad;
        pose[24] = 30.0 * rad;
        pose[25] = 0.0 * rad;
        pose[26] = 0.0 * rad;

    }
    return pose;
}

