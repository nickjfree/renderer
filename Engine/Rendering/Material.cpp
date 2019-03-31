#include "Material.h"
#include "Resource\ResourceCache.h"


USING_ALLOCATER(Material);

Material::Material(Context * context) : Resource(context), ShaderProgram(0)
{
	ResourceType = R_MATERIAL;
}


Material::~Material()
{
}


int Material::OnSerialize(Deserializer& deserializer){
	//parse as xml file
	RawXML = deserializer.Raw();
	XMLParser Parser;
	xml_doc = Parser.Parse(RawXML);
	// free buffer used by file data
	
	// read xml structures
	xml_node<> * material = xml_doc->first_node();
	xml_node<> * node = material->first_node();
	if (!strcmp(node->name(), "textures")) {

		xml_node<> * texture = node->first_node();
		while (texture) {
			// read textures resource
			xml_attribute<> * attr = texture->first_attribute("url");
			char * url = attr->value();
			attr = texture->first_attribute("unit");
			char * Unit = attr->value();
			texture = texture->next_sibling();
			Dependencies[String(url)].as<char*>() = Unit;
			DepCount++;
			printf("texture: %s  %s\n", Unit, url);
		}
	}
	// read shader resource
	node = node->next_sibling();
	if (!strcmp(node->name(), "shader")) {
		xml_attribute<> * attr = node->first_attribute("url");
		char * url = attr->value();
		Dependencies[String(url)].as<char*>() = NULL;
		DepCount++;
		printf("shader %s\n", url);
	}
	// read shader parameters
	node = node->next_sibling();
	if (node && !strcmp(node->name(), "parameters")) {

		xml_node<> * parameter = node->first_node();
		while (parameter) {
			// read textures resource
			xml_attribute<> * attr = parameter->first_attribute("name");
			char * name = attr->value();
			attr = parameter->first_attribute("value");
			float value = static_cast<float>(atof(attr->value()));
			parameter = parameter->next_sibling();
			Parameters[String(name)].as<float>() = value;
			printf("material parameter: %s  %f\n", name, value);
		}
	}
	return 0;
}

int Material::OnCreateComplete(Variant& Parameter) {
	ResourceCache * Cache = context->GetSubsystem<ResourceCache>();
	// load sub resource
	Dict::Iterator Iter;
	for (Iter = Dependencies.Begin(); Iter != Dependencies.End(); Iter++) {
		KeyValue& kv = *Iter;
		// key is resource url
		printf("async loading %s \n", kv.key.ToStr());
		// char * tmp = (char*)kv.key;
		Cache->AsyncLoadResource(kv.key, this, kv.Value);
	}
	OwnerParameter = Parameter;
	return 0;
}



int Material::OnSubResource(int Message, Resource * Sub, Variant& Param) {
	GPUResource * resource = (GPUResource*)Sub;
	if (resource->ResourceType == R_TEXTURE) {
		char * texunit = Param.as<char*>();
		//printf("finish texture %s\n", texunit);
		Parameters[String(texunit)].as<int>() = resource->GetId();
		DepCount--;
	}
	if (resource->ResourceType == R_SHADER) {
		//printf("finish shader\n");
		ShaderProgram = (Shader*)resource;
		DepCount--;
	}
	//printf("remain depcount %d\n", DepCount);
	if (!DepCount) {
		//printf("finish material\n");
		NotifyOwner(RM_LOAD, OwnerParameter);
	}
	return 0;
}

int Material::Compile(BatchCompiler * Compiler, int Stage, int Lod) {
	return 0;
}