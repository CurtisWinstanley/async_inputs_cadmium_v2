#include <limits>
#include <string>
#include "devstone_atomic.hpp"
#include "devstone_coupled.hpp"
#include "devstone_coupled_li.hpp"
#include "devstone_coupled_hi.hpp"
#include "devstone_coupled_ho.hpp"
#include "devstone_coupled_homod.hpp"

DEVStoneCoupled::DEVStoneCoupled(int width, int depth, int intDelay, int extDelay): cadmium::Coupled("coupled" + std::to_string(depth)), childCoupled() {
	if (depth < 1 || width < 1) {
		throw std::bad_exception();  // TODO custom exceptions
	}
	addInPort<int>("in");
	addOutPort<int>("out");
	if (depth == 1) {
		auto atomic = std::make_shared<DEVStoneAtomic>("innerAtomic", intDelay, extDelay);
		addComponent(atomic);
		addCoupling(getInPort("in"), atomic->getInPort("in"));
		addCoupling(atomic->getOutPort("out"), getOutPort("out"));
	}
}

std::shared_ptr<DEVStoneCoupled> DEVStoneCoupled::newDEVStoneCoupled(const std::string& type, int width, int depth, int intDelay, int extDelay) {
	if (type == "LI") {
		return std::make_shared<DEVStoneCoupledLI>(width, depth, intDelay, extDelay);
	} else if (type == "HI") {
		return std::make_shared<DEVStoneCoupledHI>(width, depth, intDelay, extDelay);
	} else if (type == "HO") {
		return std::make_shared<DEVStoneCoupledHO>(width, depth, intDelay, extDelay);
	} else if (type == "HOmod") {
		return std::make_shared<DEVStoneCoupledHOmod>(width, depth, intDelay, extDelay);
	} else {
		throw std::bad_exception();  // TODO custom exceptions
	}
}

cadmium::RootCoordinator DEVStoneCoupled::createEngine(const std::shared_ptr<DEVStoneCoupled>& devstone) {
	auto rootCoordinator = cadmium::RootCoordinator(devstone);
	rootCoordinator.start();
	return rootCoordinator;
}
[[maybe_unused]] void DEVStoneCoupled::runSimulation(cadmium::RootCoordinator& rootCoordinator) {
	for (const auto& inPort: rootCoordinator.getTopCoordinator()->getComponent()->getInPorts().getPorts()) {
		rootCoordinator.getTopCoordinator()->inject(0, std::dynamic_pointer_cast<cadmium::Port<int>>(inPort), -1);
	}
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
}

unsigned long DEVStoneCoupled::nAtomics() const {
	auto res = components.size();
	if (childCoupled != nullptr) {
		res += childCoupled->nAtomics() - 1;
	}
	return res;
}

unsigned long DEVStoneCoupled::nEICs() const {
	auto res = EIC.size();
	if (childCoupled != nullptr) {
		res += childCoupled->nEICs();
	}
	return res;
}

unsigned long DEVStoneCoupled::nICs() const {
	auto res = IC.size();
	if (childCoupled != nullptr) {
		res += childCoupled->nICs();
	}
	return res;
}

unsigned long DEVStoneCoupled::nEOCs() const {
	auto res = EOC.size();
	if (childCoupled != nullptr) {
		res += childCoupled->nEOCs();
	}
	return res;
}

unsigned long DEVStoneCoupled::nTransitions() const {
	auto res = (childCoupled == nullptr) ? 0 : childCoupled->nTransitions();
	for (const auto& child: components) {
		auto atomic = std::dynamic_pointer_cast<DEVStoneAtomic>(child);
		if (atomic != nullptr) {
			res += atomic->nTransitions();
		}
	}
	return res;
}
