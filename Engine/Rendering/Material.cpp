#include "Material.h"
#include "Resource\ResourceCache.h"


USING_ALLOCATER(Material);

Material::Material(Context* context) : Resource(context), ShaderProgram(0)
{
	ResourceType = R_MATERIAL;
}


Material::~Material()
{
}


int Material::OnSerialize(Deserializer& deserializer) {
	//parse as xml file
	RawXML = deserializer.Raw();
	XMLParser Parser;
	xml_doc = std::unique_ptr<xml_document<>>(Parser.Parse(RawXML));
	// free buffer used by file data

	// read xml structures
	xml_node<>* material = xml_doc->first_node();
	xml_node<>* node = material->first_node();

	while (node) {
		if (!strcmp(node->name(), "textures")) {

			xml_node<>* texture = node->first_node();
			while (texture) {
				// read textures resource
				xml_attribute<>* attr = texture->first_attribute("url");
				char* url = attr->value();
				attr = texture->first_attribute("unit");
				char* unit = attr->value();
				texture = texture->next_sibling();
				Dependencies[String(url)] = unit;
				DepCount++;
				printf("texture: %s  %s\n", unit, url);
			}
		}
		// read shader resource
		if (!strcmp(node->name(), "shader")) {
			xml_attribute<>* attr = node->first_attribute("url");
			char* url = attr->value();
			Dependencies[String(url)] = nullptr;
			DepCount++;
			printf("shader %s\n", url);
		}
		// read shader library
		if (!strcmp(node->name(), "library")) {
			xml_attribute<>* attr = node->first_attribute("url");
			char* url = attr->value();
			ShaderLibrary* nullShader;
			ShaderLibs.PushBack(nullShader);
			Dependencies[String(url)] = ShaderLibs.Size() - 1;
			DepCount++;
			printf("shader %s\n", url);
		}
		// read shader parameters
		if (node && !strcmp(node->name(), "parameters")) {

			xml_node<>* parameter = node->first_node();
			while (parameter) {
				// read textures resource
				xml_attribute<>* attr = parameter->first_attribute("name");
				char* name = attr->value();
				attr = parameter->first_attribute("value");
				float value = static_cast<float>(atof(attr->value()));
				parameter = parameter->next_sibling();
				Parameters[String(name)] = value;
				printf("material parameter: %s  %f\n", name, value);
			}
		}
		// next node
		node = node->next_sibling();
	}
	return 0;
}

int Material::OnCreateComplete(Variant& Parameter) {
	ResourceCache* Cache = context->GetSubsystem<ResourceCache>();
	// load sub resource
	Dict::Iterator Iter;
	for (Iter = Dependencies.Begin(); Iter != Dependencies.End(); Iter++) {
		KeyValue& kv = *Iter;
		// key is resource url
		printf("async loading %s \n", kv.key.ToStr());
		// char * tmp = (char*)kv.key;
		Cache->AsyncLoadResource(kv.key, this, kv.Value);
	}
	return 0;
}



int Material::OnSubResource(int Message, Resource* Sub, Variant& Param) {
	GPUResource* resource = (GPUResource*)Sub;
	if (resource->ResourceType == R_TEXTURE) {
		char* texunit = Param.as<char*>();
		//printf("finish texture %s\n", texunit);
		Parameters[String(texunit)] = resource->GetId();
		// add to texture bindings
		auto texture = Material::TextureUnit{};
		texture.resourceId = resource->GetId();
		texture.slot = GetShaderBindingSlot(texunit);
		if (texture.slot != -1) {
			Textures.PushBack(texture);
		}
		DepCount--;
	}
	if (resource->ResourceType == R_SHADER) {
		//printf("finish shader\n");
		ShaderProgram = (Shader*)resource;
		DepCount--;
	}
	if (resource->ResourceType == R_SHADER_LIBRARY) {
		//printf("finish shader\n");
		ShaderLibs[Param.as<int>()] = (ShaderLibrary*)resource;
		DepCount--;
	}
	//printf("remain depcount %d\n", DepCount);
	return 0;
}

void Material::Apply(RenderCommandContext* cmdContext)
{
	for (auto iter = Textures.Begin(); iter != Textures.End(); iter++) {
		auto& texture = *iter;
		cmdContext->SetSRV(texture.slot, texture.resourceId);
	}
}

int Material::OnDestroy(Variant& Data) {
	// unload all textures 
	auto cache = context->GetSubsystem<ResourceCache>();
	for (auto iter = Dependencies.Begin(); iter != Dependencies.End(); iter++) {
		auto kv = *iter;
		cache->AsyncUnLoadResource(kv.key, this, Data);
	}
	// free xml node
	xml_doc->clear();
	// delete xml_doc;
	DeSerial.Release();
	return 0;
}

int Material::GetRtShaderBindings(RenderContext* context, R_RAYTRACING_INSTANCE* instance) {
	int shaderIndex = 0;
	// each shaderlib is a hitgroup
	for (auto iter = ShaderLibs.Begin(); iter != ShaderLibs.End(); iter++, shaderIndex++) {
		auto rtShader = *iter;
		int bindingsPerShader = 0;
		// set shaderId
		instance->ShaderBindings[shaderIndex].ShaderId = rtShader->GetId();
		// set materials
		instance->ShaderBindings[shaderIndex].NumBindings = 0;
		auto index = 0;
		for (auto iter = Textures.Begin(); iter != Textures.End(); iter++) {
			auto& texture = *iter;
			auto& rtbinding = instance->ShaderBindings[shaderIndex].Bindings[index++];
			rtbinding.BindingType = R_SRV_TEXTURE;
			rtbinding.Slot = MATERIAL_RT_SLOT(texture.slot);
			rtbinding.ResourceId = texture.resourceId;
		}
		instance->ShaderBindings[shaderIndex].NumBindings = index - 1;
		// inc numShaders
		++instance->NumShaders;
	}
	return instance->NumShaders;
}