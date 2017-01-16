#include "Shader.h"
#include "Resource\XMLParser.h"
#include "D3Dcompiler.h"


R_FORMAT GetFormat(int Mask, int Type) {
	R_FORMAT format;
	// and types
	if (Mask == 1) {
		switch (Type) {
		case D3D_REGISTER_COMPONENT_UINT32:
			format = FORMAT_R32_UINT;
			break;
		case D3D_REGISTER_COMPONENT_SINT32:
			format = FORMAT_R32_SINT;
			break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
			format = FORMAT_R32_FLOAT;
			break;
		default:
			// error
			break;
		}
	}
	else if (Mask == 3) {
		switch (Type) {
		case D3D_REGISTER_COMPONENT_UINT32:
			format = FORMAT_R32G32_UINT;
			break;
		case D3D_REGISTER_COMPONENT_SINT32:
			format = FORMAT_R32G32_SINT;
			break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
			format = FORMAT_R32G32_FLOAT;
			break;
		default:
			// error
			break;
		}
	}
	else if (Mask == 7) {
		switch (Type) {
		case D3D_REGISTER_COMPONENT_UINT32:
			format = FORMAT_R32G32B32_UINT;
			break;
		case D3D_REGISTER_COMPONENT_SINT32:
			format = FORMAT_R32G32B32_SINT;
			break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
			format = FORMAT_R32G32B32_FLOAT;
			break;
		default:
			// error
			break;
		}
	}
	else if (Mask == 15) {
		switch (Type) {
		case D3D_REGISTER_COMPONENT_UINT32:
			format = FORMAT_R32G32B32A32_UINT;
			break;
		case D3D_REGISTER_COMPONENT_SINT32:
			format = FORMAT_R32G32B32A32_SINT;
			break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
			format = FORMAT_R32G32B32A32_FLOAT;
			break;
		default:
			// error
			break;
		}
	}
	return format;
}

int GetOffset(int Mask) {
	switch (Mask) {
	case 1:
		return 4;
	case 3:
		return 8;
		break;
	case 7:
		return 12;
		break;
	case 15:
		return 16;
	default:
		return 0;
	}
	return 0;
}



USING_ALLOCATER(Shader);

Shader::Shader(Context * context): GPUResource(context), RawXML(0) {
	ResourceType = R_SHADER;
}

Shader::~Shader()
{
}

int Shader::Apply(int Stage, char * buffer, Dict& Parameter) {
	return 0;
}


Deserializer Shader::AsyncLoad() {
	String url = "Shader\\Shaders\\Shaders.xml\\0";
	return Loader->GetDeserializer(url);
}

int Shader::OnSerialize(Deserializer& deserializer) {
	XMLParser Parser;
	RawXML = deserializer.Raw();
	xml_doc = Parser.Parse(RawXML);
	// parse technique and pass
	xml_node<> * node;
	node = xml_doc->first_node("shaders");
	// get first shader node
	node = node->first_node();
	char * name = (char*)File;
	// find shader node with name same as File
	while (node && strcmp(node->first_attribute("name")->value(), name)) {
		node = node->next_sibling();
	}
	if (node) {
		xml_node<> * tech = node->first_node();
		while (tech) {
			Technique Tech;
			Tech.PassCount = 0;
			Tech.Name = tech->first_attribute("name")->value();
			xml_node<> * pass = tech->first_node();
			while (pass) {
				Pass * RenderPass = new Pass();
				RenderPass->VS = RenderPass->PS = RenderPass->HS = RenderPass->GS = RenderPass->DS = -1;
				RenderPass->Blend = RenderPass->DepthStencil = RenderPass->Rasterizer = -1;
				RenderPass->Name = pass->first_attribute("name")->value();
				// renderstats
				String StateName;
				StateName = pass->first_attribute("depthstencil")->value();
				RenderPass->DepthStencil = rendercontext->GetRenderState(StateName);
				StateName = pass->first_attribute("rasterizer")->value();
				RenderPass->Rasterizer = rendercontext->GetRenderState(StateName);
				StateName = pass->first_attribute("blend")->value();
				RenderPass->Blend = rendercontext->GetRenderState(StateName);
				// pars shaders
				xml_node<> * shader = pass->first_node();
				while (shader) {
					String url = shader->first_attribute("url")->value();
					FileLoader Loader(context);
					Deserializer ShaderDes = Loader.GetDeserializer(url);
					void * Binary = ShaderDes.Raw();
					unsigned int Size = ShaderDes.Length();
					if (!strcmp(shader->name(), "vs")) {
						RenderPass->VS = renderinterface->CreateVertexShader(Binary, Size, 0);
					}
					else if (!strcmp(shader->name(), "gs")) {
						RenderPass->GS = renderinterface->CreatePixelShader(Binary, Size, 0);
					}
					else if (!strcmp(shader->name(), "hs")) {
						RenderPass->HS = renderinterface->CreateHullShader(Binary, Size, 0);
					}
					else if (!strcmp(shader->name(), "ds")) {
						RenderPass->DS = renderinterface->CreateDomainShader(Binary, Size, 0);
					}
					else if (!strcmp(shader->name(), "ps")) {
						RenderPass->PS = renderinterface->CreatePixelShader(Binary, Size, 0);
					}
					// reflect shader constant and textures
					ReflectShader(RenderPass, Binary, Size);
					shader = shader->next_sibling();
				}
				Tech.RenderPass[Tech.PassCount++] = RenderPass;
				pass = pass->next_sibling();
			}
			Techs.PushBack(Tech);
			tech = tech->next_sibling();
		}
	}
	return 0;
}

int Shader::OnCreateComplete(Variant& Parameter) {
	// Creat constant buffers
	for (int i = 0; i < Techs.Size(); i++) {
		Technique& tc = Techs[i];
		for (int i = 0; i < tc.PassCount; i++) {
			Pass& pass = *tc.RenderPass[i];
			//printf("in pass %s\n", (char*)pass.Name);
			for (int i = 0; i < pass.Constants.Size(); i++) {
				ConstantBuffer& cb = pass.Constants[i];
				//printf("constant buffer name %s, slot %d, size %d\n", (char*)cb.Name, cb.Slot, cb.Size);
				// register this constant buffer
				int result = rendercontext->RegisterConstant(cb.Name, cb.Slot, cb.Size);
			}
			for (int i = 0; i < pass.Parameters.Size(); i++) {
				ShaderParameter& sp = pass.Parameters[i];
				//printf("shader parameter name %s, slot %d, offset %d, size %d, in constant %s\n", (char*)sp.Name, sp.Slot, sp.Offset, sp.Size, (char*)sp.Constant);
			}
			for (int i = 0; i < pass.TextureUnits.Size(); i++) {
				TextureUnit& tu = pass.TextureUnits[i];
				Variant * texture = rendercontext->GetResource(tu.Name);
				if (texture) {
					printf("texture unit %s, slot %d, id %d\n", (char*)tu.Name, tu.Slot, texture->as<int>());
				}
			}
			//printf("Depth %d, Rasterizer %d Blend %d\n", pass.DepthStencil, pass.Rasterizer, pass.Blend);
		}
	}
	NotifyOwner(RM_LOAD, Parameter);
	// create the constant buffer and register shader parameters
	return 0;
}

int Shader::OnLoadComplete(Variant& Parameter) {
	// do nothing for shader resource
	return 0;
}

int Shader::ReflectShader(Pass * RenderPass, void * Shader, unsigned int Size) {
	/*
		shader reflection to get contant buffer info and input signature info
	*/
	ID3D11ShaderReflection * Reflector;
	D3DReflect(Shader, Size, IID_ID3D11ShaderReflection, (void **)&Reflector);
	// get shader desc
	D3D11_SHADER_DESC desc;
	Reflector->GetDesc(&desc);
	// build input signature if it is a vertex shader
	if (D3D11_SHVER_GET_TYPE(desc.Version) == D3D11_SHVER_VERTEX_SHADER) {
		// build the inputlayout
		R_INPUT_ELEMENT Elements[32];  // 32 is enough for now
		int Offset = 0;
		memset(Elements, 0, sizeof(R_INPUT_ELEMENT) * 32);
		for (int i = 0; i < desc.InputParameters; i++) {
			D3D11_SIGNATURE_PARAMETER_DESC input_desc;
			Reflector->GetInputParameterDesc(i, &input_desc);
			R_INPUT_ELEMENT * Element = &Elements[i];
			Element->Semantic = (char*)input_desc.SemanticName;
			Element->SemanticIndex = input_desc.SemanticIndex;
			Element->Slot = input_desc.Stream;
			Element->Offset = Offset;
			Element->Format = GetFormat(input_desc.Mask, input_desc.ComponentType);
			Offset += GetOffset(input_desc.Mask);
		}
		// create input layout
		RenderPass->InputLayout = renderinterface->CreateInputLayout(Elements, desc.InputParameters, Shader, Size);
	}

	// get constant buffers
	for (int i = 0; i < desc.ConstantBuffers; i++) {
		D3D11_SHADER_BUFFER_DESC Description;
		ID3D11ShaderReflectionConstantBuffer* ConstBuffer = Reflector->GetConstantBufferByIndex(i);
		ConstBuffer->GetDesc(&Description);
		// get constant buffer bind
		D3D11_SHADER_INPUT_BIND_DESC bind_desc;
		Reflector->GetResourceBindingDescByName(Description.Name, &bind_desc);
		// store constant buffer info
		ConstantBuffer cb;
		cb.Name = (char*)Description.Name;
		cb.Size = Description.Size;
		cb.Slot = bind_desc.BindPoint;
		RenderPass->Constants.PushBack(cb);
		for (int j = 0; j < Description.Variables; j++)  {   // Get the variable description and store it   
			ID3D11ShaderReflectionVariable* Variable = ConstBuffer->GetVariableByIndex(j);
			D3D11_SHADER_VARIABLE_DESC var_desc;   
			Variable->GetDesc( &var_desc );     
			ID3D11ShaderReflectionType* pType = Variable->GetType();   
			D3D11_SHADER_TYPE_DESC type_desc;
			pType->GetDesc( &type_desc );
			// store shader parameter info
			ShaderParameter sp;
			sp.Offset = var_desc.StartOffset;
			sp.Slot = cb.Slot;
			sp.Name = (char*)var_desc.Name;
			sp.Size = var_desc.Size;
			sp.Constant = (char*)cb.Name;
			RenderPass->Parameters.PushBack(sp);
		}
	}
	// get textures
	for (int i = 0; i < desc.BoundResources; i++) {
		// get constant buffer bind
		D3D11_SHADER_INPUT_BIND_DESC bind_desc;
		Reflector->GetResourceBindingDesc(i, &bind_desc);
		if (bind_desc.Type == D3D_SIT_TEXTURE) {
			// store constant buffer info
			TextureUnit tu;
			tu.Name = (char*)bind_desc.Name;
			tu.Slot = bind_desc.BindPoint;
			RenderPass->TextureUnits.PushBack(tu);
		}
	}

	return 0;
}

Variant * Shader::GetParameter(String& Name, Dict& Material, Dict& Object, RenderContext * Context) {
	Dict::Iterator Iter = Material.Find(Name);
	if (Iter != Material.End()) {
		return &(*Iter).Value;
	}
	Iter = Object.Find(Name);
	if (Iter != Object.End()) {
		return &(*Iter).Value;
	}
	return rendercontext->GetResource(Name);
}

int Shader::Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& MaterialParam, Dict& ObjectParameter, RenderContext * Context) {
	int techs = Techs.Size();
	int Compiled = 0;
	if (techs) {
		Technique * tech = &Techs[Stage];
		for (int c = 0; c < tech->PassCount; c++) {
			Pass * pass = tech->RenderPass[c];
			// inputayput
			Compiled += Compiler->SetInputLayout(pass->InputLayout);
			// textures
			int texture_units = pass->TextureUnits.Size();
			for (int i = 0; i < texture_units; i++) {
				TextureUnit * unit = &pass->TextureUnits[i];
				Variant * Value = GetParameter(unit->Name, MaterialParam, ObjectParameter, Context);
				if (Value) {
					int id = Value->as<int>();
					Compiled += Compiler->SetTexture(unit->Slot, id);
				}
			}
			// constants
			int parameters = pass->Parameters.Size();
			for (int i = 0; i < parameters; i++) {
				ShaderParameter * parameter = &pass->Parameters[i];
				Variant * Value = GetParameter(parameter->Name, MaterialParam, ObjectParameter, Context);
				if (Value) {
					Compiled += Compiler->SetShaderParameter(parameter->Slot, parameter->Offset, parameter->Size, Value);
				}
			}
			// update constants
			int Constants = pass->Constants.Size();
			for (int i = 0; i < Constants; i++) {
				ConstantBuffer * cons = &pass->Constants[i];
				Compiled += Compiler->UpdateConstant(cons->Slot);
			}
			// shaders
			if (pass->VS != -1)
				Compiled += Compiler->SetVertexShader(pass->VS);
			if (pass->GS != -1)
				Compiled += Compiler->RenderGeometry(pass->GS);
			if (pass->HS != -1)
				Compiled += Compiler->SetHullShader(pass->HS);
			if (pass->DS != -1)
				Compiled += Compiler->SetDomainShader(pass->DS);
			if (pass->PS != -1)
				Compiled += Compiler->SetPixelShader(pass->PS);
			// render stats
			Compiled += Compiler->SetDepthStencil(pass->DepthStencil);
			Compiled += Compiler->SetRasterizer(pass->Rasterizer);
			Compiled += Compiler->SetBlend(pass->Blend);
		}
	}
	return Compiled;
}
