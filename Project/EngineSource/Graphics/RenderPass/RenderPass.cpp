#include"RenderPass.h"

using namespace GameEngine;

RenderPass::RenderPass(const std::string& name,RenderPass* renderPass) {
	name_ = name;
	renderPass_ = renderPass;
}

void RenderPass::PrePass() {

}

void RenderPass::PostPass() {

}