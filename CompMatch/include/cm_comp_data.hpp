#pragma once
#ifndef CM_COMP_DATA_HPP
#define CM_COMP_DATA_HPP
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "cm_comp_data.hpp"


using json = nlohmann::json;

namespace cm {
	enum ComponentType {
		BOX_COMPONENT = 0,
		LEAD_COMPONENT = 1,
		BALL_GROUP_COMPONENT = 2,
	};

	class CommonData {
	public:
		CommonData();
		~CommonData();

		void SetComponentLenth(double lenth);
		void SetComponentWidth(double width);
		
		double GetComponentLenth();
		double GetComponentWidth();

	private:
		double length_;									//元件长
		double width_;									//元件宽	
	};

	enum class ConfigurationBasicType {
		MOLD = 0,			    // 模具
		LEAD_GROUP = 1,         // 引脚组
		BALL_GROUP = 2,			// 球组
	};

	class AbstractConfigurationBasic {
	public:
		AbstractConfigurationBasic();
		virtual ~AbstractConfigurationBasic() = default;

		ConfigurationBasicType GetComponentBasicType() const;
		void SetComponentBasicType(ConfigurationBasicType component_basic_type);

		virtual int ParamCount();
		virtual void DeleteParam(int index);

		virtual AbstractConfigurationBasic* clone() const {
			return new AbstractConfigurationBasic(*this);
		}

		virtual AbstractConfigurationBasic* detach(int num = 0) const {
			return new AbstractConfigurationBasic(*this);
		}

		virtual void join(AbstractConfigurationBasic* basic) {
			int test = 0;
		}

	private:
		ConfigurationBasicType component_basic_type_;
	};

	class Component {
	public:
		Component();
		~Component();
		Component& operator=(Component& ref);

		using ConfigTypeMap = std::map<ConfigurationBasicType, std::shared_ptr<AbstractConfigurationBasic>>;
		using ConfigTypePtr = std::map<ConfigurationBasicType, std::shared_ptr<AbstractConfigurationBasic>>&;

		void SetComponentType(ComponentType component_type);
		void SetCommonData(CommonData common_data);

		ComponentType GetComponentType();
		CommonData& GetCommonData();

		void AddAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type, std::shared_ptr<AbstractConfigurationBasic> configuration_basic_ptr);
		void UpdateAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type, std::shared_ptr<AbstractConfigurationBasic> configuration_basic_ptr);
		void DeleteALLAbstractConfigurationBasic();
		void DeleteAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type);
		bool FindAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type);
		void UpdateConfigTypeMap(ConfigTypeMap list);
		ConfigTypeMap GetConfigurationMap();
		ConfigTypeMap GetConfigurationMapCopy();
		std::shared_ptr<AbstractConfigurationBasic> GetOneAbstractConfigurationBasic(ConfigurationBasicType configuration_basic_type);
	private:
		ComponentType component_type_;					//元件类型
		CommonData common_data_;						//元件公共数据
		ConfigTypeMap configuration_basic_list_;		//元件数据
	};

	class MoldParam {
	public:
		struct ShapeParam {
			bool shape = 0;			//形状		0-长方形 1-圆形
			double length = 0;		//特征点长
			double width = 0;		//特征点宽 
			double center_x = 0;	//中心X
			double center_y = 0;	//中心Y
		};

		MoldParam();
		~MoldParam();

		// 形状尺寸参数
		void SetShapeParam(ShapeParam shape_param);
		ShapeParam GetShapeParam();

	private:
		ShapeParam shape_param_;
	};

	class MoldBasic : public AbstractConfigurationBasic {
	public:
		MoldBasic();
		~MoldBasic() override;

		int ParamCount();
		MoldParam& GetParam();
		void AddParam(MoldParam param);
		void DeleteParam(int index);
		void ClearParam();

		MoldBasic* clone() const {
			return new MoldBasic(*this);
		}

		MoldBasic* detach(int num = 0) const {
			MoldBasic* ret = new MoldBasic();
			ret->AddParam(this->mold_param_[num]);
			return ret;
		}

		void join(AbstractConfigurationBasic* basic) {
			auto ptr = dynamic_cast<MoldBasic*>(basic);
			this->mold_param_.insert(this->mold_param_.end(), ptr->mold_param_.begin(), ptr->mold_param_.end());
		}
	private:
		std::vector<MoldParam> mold_param_;
	};

	class CutLeadParam {
	public:
		struct ACutLeadParam {
			int group = 0;       // 缺失类型
			int number = 0;      // 引脚个数
			int position = 0;    // 起始位置
		};

		CutLeadParam();
		~CutLeadParam();

		// 单个缺失引脚参数
		ACutLeadParam GetACutLeadParam(int index);
		std::vector<ACutLeadParam> GetAllCutLeadParam();
		void UpdateACutLeadParam(int index, ACutLeadParam a_cut_lead_param);
		void AddACutLeadParam(ACutLeadParam a_cut_lead_param);

	private:
		std::vector<ACutLeadParam> cut_lead_param_;
	};

	class LeadGroupParam {
	public:
		struct ShapeParam {
			int number;					//引脚数量
			double pitch;				//引脚间距
			double width;				//引脚宽度
			double length;				//引脚长度
			double angle;				//角度
			double center_x;			//中心X
			double center_y;			//中心Y	
		};

		LeadGroupParam();
		~LeadGroupParam();

		// 形状尺寸参数
		void SetShapeParam(ShapeParam shape_param);
		ShapeParam GetShapeParam();

		// 缺失引脚参数
		bool ExistCutLead();
		void SetCutLeadParam(CutLeadParam cut_lead_param);
		CutLeadParam GetCutLeadParam();
	private:
		ShapeParam shape_param_;
		CutLeadParam cut_lead_param_;
	};

	class LeadGroupBasic : public AbstractConfigurationBasic {
	public:
		LeadGroupBasic();
		~LeadGroupBasic() override;

		int ParamCount();
		LeadGroupParam& GetParam(int index);
		void AddParam(LeadGroupParam param);
		void DeleteParam(int index);
		void ClearParam();

		LeadGroupBasic* clone() const {
			return new LeadGroupBasic(*this);
		}

		LeadGroupBasic* detach(int num = 0) const {
			LeadGroupBasic* ret = new LeadGroupBasic();
			ret->AddParam(this->lead_group_param_[num]);
			return ret;
		}

		void join(AbstractConfigurationBasic* basic) {
			auto ptr = dynamic_cast<LeadGroupBasic*>(basic);
			this->lead_group_param_.insert(this->lead_group_param_.end(), ptr->lead_group_param_.begin(), ptr->lead_group_param_.end());
		}
	private:
		std::vector<LeadGroupParam> lead_group_param_;
	};

	class LackBallParam {
	public:
		struct LackParam {
			int start_row = 0;	//始行
			int start_col = 0;	//始列
			int last_row = 0;	//末行
			int last_col = 0;	//末列

			LackParam() {}
			LackParam(int s_r, int s_c, int l_r, int l_c) :start_row(s_r), start_col(s_c), last_row(l_r), last_col(l_c) {}
		};

		LackBallParam();
		~LackBallParam();

		LackParam GetLackParam(int index);
		std::vector<LackParam> GetAllLackParam();
		void AddLackParam(LackParam lack_param);
		void UpdateLackParam(int index, LackParam lack_param);
		void SetAllLackParam(std::vector<LackParam> lack_param_list);

	private:
		std::vector<LackParam> lack_param_list_;
	};

	class BallGroupParam {
	public:
		struct ShapeParam {
			double angle;				//角度
			double center_x;			//中心X
			double center_y;			//中心Y
			int number_x;				//球数X
			int number_y;				//球数Y
			double pitch_x;				//球间距X
			double pitch_y;				//球间距Y
			double diameter;			//球直径
		};

		BallGroupParam();
		~BallGroupParam();

		// 形状尺寸参数
		void SetShapeParam(ShapeParam shape_param);
		ShapeParam GetShapeParam();

		// 界面设置参数接口
		void SetNumberX(int number_x);
		void SetNumberY(int number_y);
		void SetAngle(double angle);
		void SetCenterX(double center_x);
		void SetCenterY(double center_y);
		void SetPitchX(double pitch_x);
		void SetPitchY(double pitch_y);
		void SetDiameter(double diameter);

		// 缺失焊球参数
		bool ExistLackBall();
		void SetLackBallParam(LackBallParam lack_ball_param);
		LackBallParam GetLackBallParam();

	private:
		ShapeParam shape_param_;
		LackBallParam lack_ball_param_;
	};

	class BallGroupBasic : public AbstractConfigurationBasic {
	public:
		BallGroupBasic();
		~BallGroupBasic() override;

		int ParamCount();
		BallGroupParam& GetParam(int index);
		void AddParam(BallGroupParam param);
		void DeleteParam(int index);
		void ClearParam();

		BallGroupBasic* clone() const {
			return new BallGroupBasic(*this);
		}

		BallGroupBasic* detach(int num = 0) const {
			BallGroupBasic* ret = new BallGroupBasic();
			ret->AddParam(this->ball_group_param_[num]);
			return ret;
		}

		void join(AbstractConfigurationBasic* basic) {
			auto ptr = dynamic_cast<BallGroupBasic*>(basic);
			this->ball_group_param_.insert(this->ball_group_param_.end(), ptr->ball_group_param_.begin(), ptr->ball_group_param_.end());
		}
	private:
		std::vector<BallGroupParam> ball_group_param_;
	};


	// 读取json文件
	inline std::shared_ptr<Component> GetComponentPtr(const std::string& inJsonPath) {
		std::ifstream fileJson(inJsonPath);

		json jComponent;
		fileJson >> jComponent;

		auto pComp = std::make_shared<Component>();

		if (jComponent.contains("component_type_")) {
			int typeVal = jComponent["component_type_"].get<int>();
			pComp->SetComponentType(static_cast<ComponentType>(typeVal));
		}

		if (jComponent.contains("common_data_")) {
			auto& jCommon = jComponent["common_data_"];
			CommonData commonData;
			if (jCommon.contains("length_")) {
				commonData.SetComponentLenth(jCommon["length_"].get<double>());
			}
			if (jCommon.contains("width_")) {
				commonData.SetComponentWidth(jCommon["width_"].get<double>());
			}
			pComp->SetCommonData(commonData);
		}

		if (jComponent.contains("configuration_basic_list_") && jComponent["configuration_basic_list_"].is_array()) {
			auto& jConfigList = jComponent["configuration_basic_list_"];

			for (const auto& jItem : jConfigList) {
				if (!jItem.contains("ConfigurationBasicType") || !jItem.contains("AbstractConfigurationBasic")) {
					continue;
				}

				int basicTypeVal = jItem["ConfigurationBasicType"].get<int>();
				ConfigurationBasicType basicType = static_cast<ConfigurationBasicType>(basicTypeVal);
				const auto& jAbstract = jItem["AbstractConfigurationBasic"];

				std::shared_ptr<AbstractConfigurationBasic> pBasic = nullptr;

				switch (basicType) {
				case ConfigurationBasicType::MOLD: {
					auto pMold = std::make_shared<MoldBasic>();
					if (jAbstract.contains("mold_param_") && jAbstract["mold_param_"].is_array()) {
						for (const auto& jMoldParamItem : jAbstract["mold_param_"]) {
							MoldParam moldParam;
							if (jMoldParamItem.contains("shape_param_")) {
								auto& jShape = jMoldParamItem["shape_param_"];
								MoldParam::ShapeParam shape;
								shape.shape = jShape.value("shape", false);
								shape.length = jShape.value("length", 0.0);
								shape.width = jShape.value("width", 0.0);
								shape.center_x = jShape.value("center_x", 0.0);
								shape.center_y = jShape.value("center_y", 0.0);
								moldParam.SetShapeParam(shape);
							}
							pMold->AddParam(moldParam);
						}
					}
					pBasic = pMold;
					break;
				}
				case ConfigurationBasicType::LEAD_GROUP: {
					auto pLead = std::make_shared<LeadGroupBasic>();
					if (jAbstract.contains("lead_group_param_") && jAbstract["lead_group_param_"].is_array()) {
						for (const auto& jLeadParamItem : jAbstract["lead_group_param_"]) {
							LeadGroupParam leadParam;

							if (jLeadParamItem.contains("shape_param_")) {
								auto& jShape = jLeadParamItem["shape_param_"];
								LeadGroupParam::ShapeParam shape;
								shape.number = jShape.value("number", 0);
								shape.pitch = jShape.value("pitch", 0.0);
								shape.width = jShape.value("width", 0.0);
								shape.length = jShape.value("length", 0.0);
								shape.angle = jShape.value("angle", 0.0);
								shape.center_x = jShape.value("center_x", 0.0);
								shape.center_y = jShape.value("center_y", 0.0);
								leadParam.SetShapeParam(shape);
							}

							if (jLeadParamItem.contains("cut_lead_param_")) {
								auto& jCut = jLeadParamItem["cut_lead_param_"];
								CutLeadParam cutParam;
								if (jCut.contains("cut_lead_param_") && jCut["cut_lead_param_"].is_array()) {
									for (const auto& jACut : jCut["cut_lead_param_"]) {
										CutLeadParam::ACutLeadParam aCut;
										aCut.group = jACut.value("group", 0);
										aCut.number = jACut.value("number", 0);
										aCut.position = jACut.value("position", 0);
										cutParam.AddACutLeadParam(aCut);
									}
								}
								leadParam.SetCutLeadParam(cutParam);
							}

							pLead->AddParam(leadParam);
						}
					}
					pBasic = pLead;
					break;
				}
				case ConfigurationBasicType::BALL_GROUP: {
					auto pBall = std::make_shared<BallGroupBasic>();
					if (jAbstract.contains("ball_group_param_") && jAbstract["ball_group_param_"].is_array()) {
						for (const auto& jBallParamItem : jAbstract["ball_group_param_"]) {
							BallGroupParam ballParam;

							if (jBallParamItem.contains("ShapeParam")) {
								auto& jShape = jBallParamItem["ShapeParam"];
								BallGroupParam::ShapeParam shape;
								shape.angle = jShape.value("angle", 0.0);
								shape.center_x = jShape.value("center_x", 0.0);
								shape.center_y = jShape.value("center_y", 0.0);
								shape.number_x = jShape.value("number_x", 0);
								shape.number_y = jShape.value("number_y", 0);
								shape.pitch_x = jShape.value("pitch_x", 0.0);
								shape.pitch_y = jShape.value("pitch_y", 0.0);
								shape.diameter = jShape.value("diameter", 0.0);
								ballParam.SetShapeParam(shape);
							}

							if (jBallParamItem.contains("LackBallParam")) {
								auto& jLack = jBallParamItem["LackBallParam"];
								LackBallParam lackParam;
								if (jLack.contains("lack_param_list_") && jLack["lack_param_list_"].is_array()) {
									for (const auto& jLackItem : jLack["lack_param_list_"]) {
										LackBallParam::LackParam lp;
										lp.start_row = jLackItem.value("start_row", 0);
										lp.start_col = jLackItem.value("start_col", 0);
										lp.last_row = jLackItem.value("last_row", 0);
										lp.last_col = jLackItem.value("last_col", 0);
										lackParam.AddLackParam(lp);
									}
								}
								ballParam.SetLackBallParam(lackParam);
							}

							pBall->AddParam(ballParam);
						}
					}
					pBasic = pBall;
					break;
				}
				default:
					continue;
				}

				if (pBasic) {
					pComp->AddAbstractConfigurationBasic(basicType, pBasic);
				}
			}
		}

		return pComp;
	}
}

#endif
