#include <iostream>
#include <dart/dart.hpp>
#include <dart/gui/gui.hpp>
#include <dart/utils/utils.hpp>
#include "Tendon.hpp"
#include "IkSolver.hpp"

class Controller{
public:
	//Constructor
	Controller(const SkeletonPtr& finger, std::vector<Tendon*> tendon);
	void jointControlSetter();
	void setTargetPosition(const Eigen::VectorXd& pose);
	void clearForces();
	void addSPDForces();
	void addPDForces();
	void addSPDTendonForces();
	void addSPDTendonDirectionForces();
	Eigen::VectorXd grabOrOpen(Eigen::VectorXd originalPose,bool isGrabbed);
	double prevTorque(const Eigen::Vector3d current_point);


public:
	//Target positions for the PD controllers
	Eigen::VectorXd mTargetPositions;

protected:
	//Finger model used to control
	SkeletonPtr mFinger;

	//Tendon model used to control
	std::vector<Tendon*> mTendon;
	
	//Joint force to the fingers
	Eigen::VectorXd mForces;

	Eigen::VectorXd Ycontroller;

	//Tendon force to the fingers
	Eigen::VectorXd mTendonForces;

	//control gains for proportional error terms in the PD controller
	Eigen::MatrixXd mKp;

	//Control gains for the derivative error terms in the PD controller
	Eigen::MatrixXd mKd;

	

	std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> prevForces;
};