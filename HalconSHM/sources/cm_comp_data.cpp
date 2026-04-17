#include "cm_comp_data.hpp"


using namespace cm;

// CommonData
CommonData::CommonData() {
	length_ = 0.000;
	width_ = 0.000;
}

CommonData::~CommonData() {
}

void CommonData::SetComponentLenth(double lenth) {
	length_ = lenth;
}

void CommonData::SetComponentWidth(double width) {
	width_ = width;
}

double CommonData::GetComponentLenth() {
	return length_;
}

double CommonData::GetComponentWidth() {
	return width_;
}


// AbstractConfigurationBasic
AbstractConfigurationBasic::AbstractConfigurationBasic() {
	SetComponentBasicType(ConfigurationBasicType::MOLD);
}

ConfigurationBasicType AbstractConfigurationBasic::GetComponentBasicType() const {
	return component_basic_type_;
}

void AbstractConfigurationBasic::SetComponentBasicType(ConfigurationBasicType component_basic_type) {
	component_basic_type_ = component_basic_type;
}

int AbstractConfigurationBasic::ParamCount() {
	return 0;
}

void AbstractConfigurationBasic::DeleteParam(int index) {
}


// Component
Component::Component() {
}

Component::~Component() {
}

Component& Component::operator=(Component& ref) {
	//common_data_ = ref.common_data_;
	//machine_specific_data_ = ref.machine_specific_data_;
	return *this;
}

void Component::SetComponentType(ComponentType component_type) {
	component_type_ = component_type;
}

void Component::SetCommonData(CommonData common_data) {
	common_data_ = common_data;
}

ComponentType Component::GetComponentType() {
	return component_type_;
}

CommonData& Component::GetCommonData() {
	return common_data_;
}

void Component::AddAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type, std::shared_ptr<AbstractConfigurationBasic> configuration_basic_ptr) {
	configuration_basic_ptr->SetComponentBasicType(configuration_basic_type);
	if (configuration_basic_list_.find(configuration_basic_type) != configuration_basic_list_.end()) {
		configuration_basic_list_[configuration_basic_type] = configuration_basic_ptr;
	}
	else {
		configuration_basic_list_.insert(std::pair<ConfigurationBasicType, std::shared_ptr<AbstractConfigurationBasic>>(configuration_basic_type, configuration_basic_ptr));
	}
}

void Component::UpdateAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type, std::shared_ptr<AbstractConfigurationBasic> configuration_basic_ptr) {
	configuration_basic_ptr->SetComponentBasicType(configuration_basic_type);
	configuration_basic_list_[configuration_basic_type] = configuration_basic_ptr;
}

void Component::DeleteALLAbstractConfigurationBasic() {
	std::map<ConfigurationBasicType, std::shared_ptr<AbstractConfigurationBasic>>().swap(configuration_basic_list_);
}

void Component::DeleteAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type) {
	if (configuration_basic_list_.find(configuration_basic_type) != configuration_basic_list_.end()) {
		configuration_basic_list_.erase(configuration_basic_list_.find(configuration_basic_type));
	}
}

bool Component::FindAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type) {
	if (configuration_basic_list_.find(configuration_basic_type) != configuration_basic_list_.end()) {
		return true;
	}
	return false;
}

void Component::UpdateConfigTypeMap(ConfigTypeMap list) {
	configuration_basic_list_.clear();
	configuration_basic_list_ = list;
}

Component::ConfigTypeMap Component::GetConfigurationMap() {
	return configuration_basic_list_;
}

Component::ConfigTypeMap Component::GetConfigurationMapCopy()
{
	ConfigTypeMap ret;
	for (auto iter : configuration_basic_list_) {
		ret.insert(std::make_pair(iter.first, iter.second->clone()));
	}
	return ret;
}

std::shared_ptr<AbstractConfigurationBasic> Component::GetOneAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type) {
	if (configuration_basic_list_.find(configuration_basic_type) == configuration_basic_list_.end()) {
		return nullptr;
	}
	return configuration_basic_list_.find(configuration_basic_type)->second;
}


// MoldParam
MoldParam::MoldParam() {
	shape_param_.center_x = 0;
	shape_param_.center_y = 0;
	shape_param_.length = 0;
	shape_param_.width = 0;
}

MoldParam::~MoldParam() {
}

void MoldParam::SetShapeParam(ShapeParam shape_param) {
	shape_param_ = shape_param;
}

MoldParam::ShapeParam MoldParam::GetShapeParam() {
	return shape_param_;
}


// MoldBasic
MoldBasic::MoldBasic() {
}

MoldBasic::~MoldBasic() {
}

int MoldBasic::ParamCount() {
	return mold_param_.size();
}

MoldParam& MoldBasic::GetParam() {
	return mold_param_[0];
}

void MoldBasic::AddParam(MoldParam param) {
	mold_param_.push_back(param);
}

void MoldBasic::DeleteParam(int index) {
	return;
}

void MoldBasic::ClearParam() {
	mold_param_.clear();
}


// CutLeadParam
CutLeadParam::CutLeadParam() {
}

CutLeadParam::~CutLeadParam() {
}

CutLeadParam::ACutLeadParam CutLeadParam::GetACutLeadParam(int index) {
	return cut_lead_param_[index];
}

std::vector<CutLeadParam::ACutLeadParam> CutLeadParam::GetAllCutLeadParam() {
	return cut_lead_param_;
}

void CutLeadParam::UpdateACutLeadParam(int index, ACutLeadParam a_cut_lead_param) {
	cut_lead_param_[index] = a_cut_lead_param;
}

void CutLeadParam::AddACutLeadParam(ACutLeadParam a_cut_lead_param) {
	cut_lead_param_.push_back(a_cut_lead_param);
}


// LeadGroupParam
LeadGroupParam::LeadGroupParam() {
	shape_param_.number = 0;
	shape_param_.pitch = 0;
	shape_param_.width = 0;
	shape_param_.length = 0;
	shape_param_.angle = 0;
	shape_param_.center_x = 0;
	shape_param_.center_y = 0;
}

LeadGroupParam::~LeadGroupParam() {
}

void LeadGroupParam::SetShapeParam(ShapeParam shape_param) {
	shape_param_ = shape_param;
}

LeadGroupParam::ShapeParam LeadGroupParam::GetShapeParam() {
	return shape_param_;
}

bool LeadGroupParam::ExistCutLead() {
	return false;
}

void LeadGroupParam::SetCutLeadParam(CutLeadParam cut_lead_param) {
	cut_lead_param_ = cut_lead_param;
}

CutLeadParam LeadGroupParam::GetCutLeadParam() {
	return cut_lead_param_;
}


// LeadGroupBasic
LeadGroupBasic::LeadGroupBasic() {
	SetComponentBasicType(ConfigurationBasicType::LEAD_GROUP);
}

LeadGroupBasic::~LeadGroupBasic() {
}

int LeadGroupBasic::ParamCount() {
	return lead_group_param_.size();
}

LeadGroupParam& LeadGroupBasic::GetParam(int index) {
	return lead_group_param_[index];
}

void LeadGroupBasic::AddParam(LeadGroupParam param) {
	lead_group_param_.push_back(param);
}

void LeadGroupBasic::DeleteParam(int index) {
	if (index < lead_group_param_.size()) {
		lead_group_param_.erase(lead_group_param_.begin() + index);
	}
}

void LeadGroupBasic::ClearParam() {
	lead_group_param_.clear();
}


// LackBallParam
LackBallParam::LackBallParam() {
}

LackBallParam::~LackBallParam() {
}

LackBallParam::LackParam LackBallParam::GetLackParam(int index) {
	if (index < lack_param_list_.size()) {
		return lack_param_list_[index];
	}
	return LackParam();
}

std::vector<LackBallParam::LackParam> LackBallParam::GetAllLackParam() {
	return lack_param_list_;
}

void LackBallParam::AddLackParam(LackParam lack_param) {
	lack_param_list_.push_back(lack_param);
}

void LackBallParam::UpdateLackParam(int index, LackParam lack_param) {
	if (index < lack_param_list_.size()) {
		lack_param_list_[index] = lack_param;
	}
}

void LackBallParam::SetAllLackParam(std::vector<LackParam> lack_param_list) {
	lack_param_list_ = lack_param_list;
}


// BallGroupParam
BallGroupParam::BallGroupParam() {
	shape_param_.center_x = 0;
	shape_param_.center_y = 0;
	shape_param_.diameter = 0;
	shape_param_.angle = 0;
	shape_param_.number_x = 0;
	shape_param_.number_y = 0;
	shape_param_.pitch_x = 0;
	shape_param_.pitch_y = 0;
}

BallGroupParam::~BallGroupParam() {
}

void BallGroupParam::SetShapeParam(ShapeParam shape_param) {
	shape_param_ = shape_param;
}

BallGroupParam::ShapeParam BallGroupParam::GetShapeParam() {
	return shape_param_;
}

void BallGroupParam::SetNumberX(int number_x) {
	shape_param_.number_x = number_x;
}

void BallGroupParam::SetNumberY(int number_y) {
	shape_param_.number_y = number_y;
}

void BallGroupParam::SetAngle(double angle) {
	shape_param_.angle = angle;
}

void BallGroupParam::SetCenterX(double center_x) {
	shape_param_.center_x = center_x;
}

void BallGroupParam::SetCenterY(double center_y) {
	shape_param_.center_y = center_y;
}

void BallGroupParam::SetPitchX(double pitch_x) {
	shape_param_.pitch_x = pitch_x;
}

void BallGroupParam::SetPitchY(double pitch_y) {
	shape_param_.pitch_y = pitch_y;
}

void BallGroupParam::SetDiameter(double diameter) {
	shape_param_.diameter = diameter;
}

bool BallGroupParam::ExistLackBall() {
	if (lack_ball_param_.GetAllLackParam().size() != 0) {
		return true;
	}
	return false;
}

void BallGroupParam::SetLackBallParam(LackBallParam lack_ball_param) {
	lack_ball_param_ = lack_ball_param;
}

LackBallParam BallGroupParam::GetLackBallParam() {
	return lack_ball_param_;
}


// BallGroupBasic
BallGroupBasic::BallGroupBasic() {
	SetComponentBasicType(ConfigurationBasicType::BALL_GROUP);
}

BallGroupBasic::~BallGroupBasic() {
}

int BallGroupBasic::ParamCount() {
	return ball_group_param_.size();
}

BallGroupParam& BallGroupBasic::GetParam(int index) {
	return ball_group_param_[index];
}

void BallGroupBasic::AddParam(BallGroupParam param) {
	ball_group_param_.push_back(param);
}

void BallGroupBasic::DeleteParam(int index) {
	if (index < ball_group_param_.size()) {
		ball_group_param_.erase(ball_group_param_.begin() + index);
	}
}

void BallGroupBasic::ClearParam() {
	ball_group_param_.clear();
}
