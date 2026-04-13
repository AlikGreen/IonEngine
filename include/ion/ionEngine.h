#pragma once

#include "core/engine.h"
#include "core/scene.h"

#include "input/input.h"


// components
#include "graphics/components/meshRenderer.h"
#include "graphics/components/camera.h"
#include "graphics/components/pointLight.h"
#include "audio/components/audioSource.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"

// singletons
#include "asset/assetRegistry.h"
#include "audio/audioManager.h"
#include "core/sceneManager.h"
#include "core/eventManager.h"
#include "core/resourceFs.h"

// systems
#include "core/coreSystem.h"
#include "scripting/scriptingSystem.h"
#include "audio/audioSystem.h"
#include "graphics/imGuiSystem.h"
#include "input/inputSystem.h"

// graphics
#include "graphics/image.h"

// util
#include <clogr.h>
#include <entis/entis.h>