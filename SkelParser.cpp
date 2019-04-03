#include <iostream>
#include <dart/dart.hpp>
#include <dart/gui/gui.hpp>
#include <dart/utils/utils.hpp>
#include "SkelParser.hpp"

using namespace dart::common;
using namespace dart::dynamics;
using namespace dart::simulation;
using namespace dart::gui;
using namespace dart::gui::glut;
using namespace dart::math;

 SkelParser::SkelParser(){}

 SkelParser::~SkelParser(){}
  
 void SkelParser::makeFloor(const SkeletonPtr& floor, const std::string& name){
 	//Shape
 	ShapePtr shape = std::shared_ptr<BoxShape>(new BoxShape(Eigen::Vector3d(20.0,0.5,20.0)));
	//Inertia
	double mass = default_mass;
	dart::dynamics::Inertia inertia;
	inertia.setMass(mass);
	inertia.setMoment(shape->computeInertia(mass));

	//Joint Parsing
	BodyNode* bn;
	WeldJoint::Properties props;
	props.mName = name;
	Eigen::Isometry3d T;
	T.setIdentity();	
	T.translation() = Eigen::Vector3d(0.0,0.25,0.0);
	props.mT_ChildBodyToJoint = T;
	bn = floor->createJointAndBodyNodePair<WeldJoint>(nullptr,props,BodyNode::AspectProperties(name)).second;
	bn->createShapeNodeWith<VisualAspect,CollisionAspect,DynamicsAspect>(shape);
	auto visualShapenodes = bn->getShapeNodesWith<VisualAspect>();
	visualShapenodes[0]->getVisualAspect()->setColor(dart::Color::White());
	bn->setInertia(inertia);	
 }

 void SkelParser::makeFinger(const SkeletonPtr& skel){
 	BodyNode* bn = SkelParser::makeRoot(skel, "L1");
 	bn = SkelParser::makeBallJoint(skel, bn,"L2");
 	bn = SkelParser::makeRevoluteJoint(skel, bn, "L3");
 	bn = SkelParser::makeRevoluteJoint(skel, bn, "L4");
 }
 

 void SkelParser::setGeometry(const SkeletonPtr& skel, const std::string& name, int degree, int dof){
 	int index_degree = skel->getIndexOf(skel->getBodyNode(name)->getParentJoint()->getDof(dof));
 	skel->setPosition(index_degree, degree * M_PI / 180);
 }

 BodyNode* SkelParser::makeRoot(const SkeletonPtr& skel, const std::string& name){
 	ShapePtr shape = std::shared_ptr<BoxShape>(new BoxShape(Eigen::Vector3d(default_width, default_link_len, default_width)));
	double mass = default_mass;
	dart::dynamics::Inertia inertia;
	inertia.setMass(mass);
	inertia.setMoment(shape->computeInertia(mass));
	BodyNode* bn;
	WeldJoint::Properties props;
	props.mName = name;
	Eigen::Isometry3d T;
	T.setIdentity();
	T.translation() = Eigen::Vector3d(0.0,-default_link_len/2,0.0);
	props.mT_ChildBodyToJoint = T;
	bn = skel->createJointAndBodyNodePair<WeldJoint>(
		nullptr,props,BodyNode::AspectProperties(name)).second;
	bn->createShapeNodeWith<VisualAspect,CollisionAspect,DynamicsAspect>(shape);
	bn->setInertia(inertia);	
	return bn;
 }

 BodyNode* SkelParser::makeBallJoint(const SkeletonPtr& skel, BodyNode* parent, const std::string& name){
 	ShapePtr shape = std::shared_ptr<BoxShape>(new BoxShape(Eigen::Vector3d(default_width, default_link_len, default_width)));
	double mass = default_mass;
	dart::dynamics::Inertia inertia;
	inertia.setMass(mass);
	inertia.setMoment(shape->computeInertia(mass));
	BodyNode* bn;
	BallJoint::Properties props;
	props.mName = name;
	props.mDampingCoefficients = Eigen::Vector3d::Constant(0.4);
	props.mPositionLowerLimits[0] = -30.0;
	props.mPositionUpperLimits[0] = 25.0;
	props.mPositionLowerLimits[1] = -0.0;
	props.mPositionUpperLimits[1] = 0.0;
	Eigen::Isometry3d T1;
	Eigen::Isometry3d T2;
	T1.setIdentity();
	T1.translation() = Eigen::Vector3d(0.0,-default_link_len/2, 0.0);

	T2.setIdentity();
	T2.translation() = Eigen::Vector3d(0.0, default_link_len/2,0.0);
	props.mT_ChildBodyToJoint = T1;
	props.mT_ParentBodyToJoint = T2;

	bn = skel->createJointAndBodyNodePair<BallJoint>(
		parent, props, BodyNode::AspectProperties(name)).second;

	bn->createShapeNodeWith<VisualAspect,CollisionAspect,DynamicsAspect>(shape);
	bn->setInertia(inertia);
	return bn;
 }


 BodyNode* SkelParser::makeBallJoint
 (const SkeletonPtr& skel, BodyNode* parent, const std::string& name, 
 	double width, double length, double z_len, double x_offset, double y_offset, double z_offset){
 	ShapePtr shape = std::shared_ptr<BoxShape>(new BoxShape(Eigen::Vector3d(width, length, z_len)));
	double mass = default_mass;
	dart::dynamics::Inertia inertia;
	inertia.setMass(mass);
	inertia.setMoment(shape->computeInertia(mass));
	BodyNode* bn;
	BallJoint::Properties props;
	props.mName = name;
	props.mDampingCoefficients = Eigen::Vector3d::Constant(0.4);
	// props.mPositionLowerLimits[0] = -30.0;
	// props.mPositionUpperLimits[0] = 25.0;
	// props.mPositionLowerLimits[1] = -0.0;
	// props.mPositionUpperLimits[1] = 0.0;
	Eigen::Isometry3d T1;
	Eigen::Isometry3d T2;

	T1.setIdentity();
	T1.translation() = Eigen::Vector3d(0.0,-length/2, 0.0);

	T2.setIdentity();
	T2.translation() = Eigen::Vector3d(x_offset, y_offset,z_offset);

	props.mT_ChildBodyToJoint = T1;
	props.mT_ParentBodyToJoint = T2;

	bn = skel->createJointAndBodyNodePair<BallJoint>(
		parent, props, BodyNode::AspectProperties(name)).second;

	bn->createShapeNodeWith<VisualAspect,CollisionAspect,DynamicsAspect>(shape);
	bn->setInertia(inertia);
	return bn;
 }

 BodyNode* SkelParser::makeRevoluteJoint(const SkeletonPtr& skel, BodyNode* parent, const std::string& name){
	ShapePtr shape = std::shared_ptr<BoxShape>(new BoxShape(Eigen::Vector3d(default_width, default_link_len, default_width)));
	double mass = default_mass;
	dart::dynamics::Inertia inertia;
	inertia.setMass(mass);
	inertia.setMoment(shape->computeInertia(mass));
		
	BodyNode* bn;
	RevoluteJoint::Properties props;
	props.mName = name;
	props.mAxis = Eigen::Vector3d::UnitZ();
	props.mPositionLowerLimits[0] = 0.0;
	props.mPositionUpperLimits[0] = 100.0 * M_PI / 180.0;
	Eigen::Isometry3d T1;
	Eigen::Isometry3d T2;
	T1.setIdentity();
	T1.translation() = Eigen::Vector3d(0.0, -default_link_len/2,0.0);
	T2.setIdentity();
	T2.translation() = Eigen::Vector3d(0.0, default_link_len/2,0.0);
	props.mT_ChildBodyToJoint = T1;
	props.mT_ParentBodyToJoint = T2;
	bn = skel->createJointAndBodyNodePair<RevoluteJoint>(
		parent,props,BodyNode::AspectProperties(name)).second;
	bn->createShapeNodeWith<VisualAspect,CollisionAspect,DynamicsAspect>(shape);
	bn->setInertia(inertia);
	return bn;
 }


  BodyNode* SkelParser::makeRevoluteJoint
  (const SkeletonPtr& skel, BodyNode* parent, const std::string& name, 
 	double width, double length, double z_len, double x_offset, double y_offset, double z_offset)
  {
 	ShapePtr shape = std::shared_ptr<BoxShape>(new BoxShape(Eigen::Vector3d(width, length, z_len)));
	double mass = default_mass;
	dart::dynamics::Inertia inertia;
	inertia.setMass(mass);
	inertia.setMoment(shape->computeInertia(mass));
		
	BodyNode* bn;
	RevoluteJoint::Properties props;
	props.mName = name;
	props.mAxis = Eigen::Vector3d::UnitZ();
	props.mPositionLowerLimits[0] = 0.0;
	props.mPositionUpperLimits[0] = 100.0 * M_PI / 180.0;
	Eigen::Isometry3d T1;
	Eigen::Isometry3d T2;
	T1.setIdentity();
	T1.translation() = Eigen::Vector3d(0.0, -length/2,0.0);
	T2.setIdentity();
	T2.translation() = Eigen::Vector3d(x_offset, y_offset,z_offset);
	props.mT_ChildBodyToJoint = T1;
	props.mT_ParentBodyToJoint = T2;
	bn = skel->createJointAndBodyNodePair<RevoluteJoint>(
		parent,props,BodyNode::AspectProperties(name)).second;
	bn->createShapeNodeWith<VisualAspect,CollisionAspect,DynamicsAspect>(shape);
	bn->setInertia(inertia);
	return bn;
 }
