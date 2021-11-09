#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <animation.h>
#include <bone.h>

class Animator
{
public:
	Animator() {
		//stub
	}

	Animator& operator=(Animator other) {
		swap(*this, other);

		m_CurrentAnimation = other.m_CurrentAnimation;

		return *this;
	}

	friend void swap(Animator& first, Animator& second) {
		using std::swap;

		//swap(first.m_CurrentAnimation, second.m_CurrentAnimation);

		swap(first.m_CurrentTime, second.m_CurrentTime);
		swap(first.m_DeltaTime, second.m_DeltaTime);

		swap(first.m_FinalBoneMatrices, second.m_FinalBoneMatrices);
	}

	Animator(const Animator &other) {
		this->m_CurrentAnimation = other.m_CurrentAnimation;
		this->m_CurrentTime = other.m_CurrentTime;
		this->m_DeltaTime = other.m_DeltaTime;

		this->m_FinalBoneMatrices = other.m_FinalBoneMatrices;
	}

	Animator(Animation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}
	Animation* m_CurrentAnimation;

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	float m_CurrentTime;
	float m_DeltaTime;

};