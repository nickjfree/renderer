
import bpy
import bmesh
import struct
import traceback
from mathutils import Matrix, Vector, Color


def mesh_triangulate(me):
    
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()


class H3DHeader(object):
    '''h3d header'''

    format_str = "=4sLL"

    def __init__(self, num_vertex=0, num_index=0):
        self.num_vertex = num_vertex
        self.num_index = num_index
        self.version = 0
        self.magic = b"H3D\x00"
        self.num_mesh = 1

    def pack(self):
        return struct.pack(self.format_str, self.magic, self.version, self.num_mesh)

    @classmethod
    def pack_size(cls):
        return struct.calcsize(cls.format_str)

class Vertex(object):
    '''vertex'''

    format_str = "=3f3f2ff3f"

    def __init__(self, position=Vector((0, 0, 0)), normal=Vector((0, 0, 0)), uv=Vector((0, 0)), tangent=Vector((0, 0, 0))):
        '''init'''

        self.position = position
        self.normal = normal
        self.uv = uv
        self.tangent = tangent
        self.index = 0

    def __str__(self):
        return "position: %s\nnormal: %s\nuv: %s\ntangent: %s\n " % (self.position, self.normal, self.uv, self.tangent)

    def __eq__(self, other):
        return self.position == other.position and \
            self.normal == other.normal and \
            self.uv == other.uv and \
            self.tangent == other.tangent

    def pack(self):

        # typedef struct h3d_vertex
        # {
        #     float x, y, z;
        #     float nx, ny, nz;
        #     float u, v;
        #     float parameter;
        #     float tx, ty, tz;
        # }h3d_vertex;
        return struct.pack(self.format_str, -self.position.x, self.position.z, -self.position.y,
            -self.normal.x, self.normal.z, -self.normal.y,
            self.uv.x, 1 - self.uv.y,
            0,
            -self.tangent.x, self.tangent.z, -self.tangent.y)

    @classmethod
    def pack_size(cls):
        return struct.calcsize(cls.format_str)


    def set_index(self, index):
        self.index = index



class Mesh(object):
    '''mesh to export'''


    def __init__(self, mesh=None):
        '''init'''
        self.mesh = mesh

        self.vertices = []

        self.indices = []

        self.uv_layer = []
        # mesh header size
        self.mesh_header_size = struct.calcsize("=LLLLLL256s")
        self.file_header_size = H3DHeader.pack_size()

        self.num_vertex = 0
        self.num_index = 0

        # used vertex
        self.used_vertex = set()


    def calc_data(self):
        '''prepare data for exporting'''

        self.mesh.calc_normals()
        self.mesh.calc_tangents()

        self.uv_layer = self.mesh.uv_layers.active.data
        # get polygon count
        for poly in self.mesh.polygons:
            for li in range(poly.loop_start, poly.loop_start + poly.loop_total):
                loop = self.mesh.loops[li]                 
                # vertex
                vertex = self.mesh.vertices[ loop.vertex_index]
                pos = vertex.co.xyz
                # uv
                uv = self.uv_layer[li].uv
                # normal
                normal = loop.normal
                # tangent
                tangent = loop.tangent
                # add to vertex buffer
                new_vertex = Vertex(position=pos, normal=normal, uv=uv, tangent=tangent)
                # is vertex already used
                vertex_id = len(self.vertices)
                max_vertex_id = len(self.vertices) - 1
                for i in range(0, len(self.vertices)):
                    old_vertex = self.vertices[max_vertex_id - i]
                    if old_vertex == new_vertex:
                        vertex_id = max_vertex_id - i
                        break
                if vertex_id == len(self.vertices):
                    # not match
                    self.vertices.append(new_vertex)
                # set index 
                self.indices.append(vertex_id)

        self.num_vertex = len(self.vertices)
        self.num_index = len(self.indices)


    def pack_mesh_header(self, offset_vertex, offset_index):

        # typedef struct h3d_mesh
        # {
        #     DWORD VertexSize;
        #     DWORD IndexSize;
        #     DWORD VertexNum;
        #     DWORD OffsetVertex;
        #     DWORD IndexNum;
        #     DWORD OffsetIndex;
        #     char  Texture[256];
        # }h3d_mesh;        
        return struct.pack("=LLLLLL256s", 
            Vertex.pack_size(), 2, self.num_vertex, offset_vertex,
            self.num_index, offset_index, b"\x00" * 256)

    def pack_file_header(self):
        return H3DHeader(num_vertex=self.num_vertex, num_index=self.num_index).pack()


    def export_h3d(self, path=""):
        '''export to h3d file'''

        # get offsets
        offset_vertex = self.file_header_size + self.mesh_header_size
        offset_index = offset_vertex + len(self.vertices) * Vertex.pack_size()

        try:
            # write to file
            print("saving to %s" % (path,))
            with open(path, "wb+") as file:
                # write headers
                file.write(self.pack_file_header())
                file.write(self.pack_mesh_header(offset_vertex, offset_index))
                # write data
                for v in self.vertices:
                    file.write(v.pack())
                for index in self.indices:
                    file.write(struct.pack("=H", index))

        except Exception as exc:
            traceback.print_exc()

    def export_console(self):
        '''export to h3d file'''

        print("vertex count", self.num_vertex)
        print("index count", self.num_index)


def export_object_mesh(obj):

    print("exrpoting ", obj.name)

    me = obj.to_mesh()
    mesh_triangulate(me)

    mesh = Mesh(mesh=me)
    mesh.calc_data()
    mesh.export_h3d(path="F:\\proj\\h3dPack\\Debug\\%s.h3d" % (obj.name,))
    mesh.export_console()


def export_mesh(me):

    print("exrpoting ", me.name)

    mesh_triangulate(me)

    mesh = Mesh(mesh=me)
    mesh.calc_data()
    mesh.export_h3d(path="F:\\proj\\h3dPack\\Debug\\%s.h3d" % (me.name,))
    mesh.export_console()


class Material(object):


    def __init__(self, mat):
        self.mat = mat


    def to_xml(self):

        xml_template = r'''
<?xml version="1.0"?>
<material>
  <textures>
    <texture unit="MATERIAL_DIFFUSE" name="diffuse" url="Texture\textures\{diffuse}.dds\0">diffuse_color</texture>
    <texture unit="MATERIAL_NORMAL" name="normal" url="Texture\textures\{normal}.dds\0">normal_map</texture>
    <texture unit="MATERIAL_SPECULAR" name="specular" url="Texture\textures\{specular}.dds\0">specular_map</texture>
  </textures> 
  
  <shader name="Basic" url="Shader\shaders\Instance\0"></shader>

  <library name="rtReflection" url="ShaderLibrary\shaders\rt-reflection.cso"></library>
  <library name="rtLighting" url="ShaderLibrary\shaders\rt-lighting.cso"></library>
  <parameters>
    <parameter name="gSpecular" value="0.04f"/>
  </parameters>
</material>
'''

        # find texture names
        textures = {}
        for link in self.mat.node_tree.links:

            if link.to_node.type not in ["BSDF_PRINCIPLED", "NORMAL_MAP"]:
                continue

            from_node = link.from_node

            if from_node.type == "TEX_IMAGE":

                if link.to_socket.name == "Base Color":
                    textures["diffuse"] = from_node.image.name

                if link.to_socket.name == "Specular":
                    textures["specular"] = from_node.image.name

                if link.to_socket.name == "Color" and link.to_node.type == "NORMAL_MAP":

                    textures["normal"] = from_node.image.name

        xml = xml_template.format(**textures)
        return xml.encode("utf-8")

    def export_to_file(self):

        path = "F:\\proj\\h3dPack\\Debug\\Materials\\%s.xml" % (self.mat.name,)
        with open(path, "wb+") as file:
            file.write(self.to_xml())
            return path


class Scene(object):

    def __init__(self):
        self.objects = []
        self.materials = []
        self.env_material_index = 0

    def pack_entity_header(self, number):   
        return struct.pack("=LLLL", number, 0, 0, 0)

    def pack_mesh(self, path):
        return struct.pack("=256s", path.encode("utf-8"))

    def pack_material(self, path):
        return struct.pack("=256s", path.encode("utf-8"))

    def pack_model(self, name, mesh_index):
        return struct.pack("=128s8L", name.encode("utf-8"), mesh_index, 0, 0, 0, 0, 0, 0, 0)

    def pack_skeletion(self, path):
        return struct.pack("=256s", path.encode("utf-8"))

    def pack_animation(self, path):
        return struct.pack("=256s", path.encode("utf-8"))

    def pack_blendshape(self, path):
        return struct.pack("=256s", path.encode("utf-8"))


    def pack_renderer(self, model_index, material_index):
        # typedef struct RenderEntry {
        #     ComponentEntry Info;
        #     int ModelIndex;              // model index
        #     int MaterialIndex;           // material index
        #     int pad1, pad2;
        # }RenderEntry;
        return struct.pack("=64s4L", b"Renderer", model_index, material_index, 0, 0)

    def pack_light(self, material_index, type, intensity, radius, color, direction):
        # typedef struct LightEntry {
        #     ComponentEntry Info;
        #     int ModelIndex;
        #     int MaterialIndex;
        #     int Type;
        #     float Intensity;
        #     float Radius;
        #     int pad0, pad1, pad2,
        #     Vector3 Color;
        #     Vector3 Direction;
        # }LightEntry;
        print(color, direction)
        return struct.pack("=64s3L2f3L4f4f", b"Light", 0, 
            material_index, 
            type, 
            intensity, 
            radius,
            0, 0, 0,
            color.x, color.y, color.z, 0,
            direction.x, direction.y, direction.z, 0)

    def pack_object_entry(self, obj):
        # char Name[128];
        # Vector3 Position;
        # Quaternion Rotation;
        # Vector3 Scale;
        # int NumComponents;

        # header = struct.pack("=128s4f4f4f4L", obj.name.encode("utf-8"), 
        #     obj.location.x, obj.location.y, obj.location.z, 1,
        #     obj.rotation_quaternion.x, obj.rotation_quaternion.y, obj.rotation_quaternion.z, obj.rotation_quaternion.w,
        #     obj.scale.x, obj.scale.y, obj.scale.z, 0,
        #     1,
        #     0, 0, 0)

        header = struct.pack("=128s4f4f4f4L", obj.name.encode("utf-8"), 
            -obj.location.x, obj.location.z, -obj.location.y, 1,
            0, 0, 0, 1,
            -obj.scale.x, obj.scale.z, -obj.scale.y, 0,
            1,
            0, 0, 0)

        # renderer component      
        # find mesh index and material index
        model_index = bpy.data.meshes.find(obj.data.name)
        material_index = self.materials.index(obj.active_material)
        renderer = self.pack_renderer(model_index, material_index)
        return header + renderer

    def pack_light_entry(self, postion, rotation, type, intensity, radius, color, direction):
        # char Name[128];
        # Vector3 Position;
        # Quaternion Rotation;
        # Vector3 Scale;
        # int NumComponents;

        header = struct.pack("=128s4f4f4f4L", b"LightProb", 
            0, 0, 0, 1,
            0, 0, 0, 1,
            1, 1, 1, 0,
            1,
            0, 0, 0)

        renderer = self.pack_light(self.env_material_index, type, intensity, radius, color, direction)
        return header + renderer

    def export_meshes(self, file, scene_name):

        number = len(bpy.data.meshes)

        file.write(self.pack_entity_header(number))

        for mesh in bpy.data.meshes:
            # export mesh to h3d files
            # export_mesh(mesh)
            # write mesh entry
            mesh_url = "Mesh\\%s.pack\\%s\\0" % (scene_name, mesh.name)
            file.write(self.pack_mesh(mesh_url))

    def export_materials(self, file):

        number = 0
        for mat in bpy.data.materials:

            if mat.use_nodes:
                number += 1
                self.materials.append(mat)

        number = len(bpy.data.materials)

        number += 1
        file.write(self.pack_entity_header(number))

        for mat in bpy.data.materials:
            if mat.use_nodes:
                material = Material(mat)
                material.export_to_file()

                url = "Material\\Materials\\%s.xml\\0" % (mat.name)
                file.write(self.pack_material(url))

        # env light 
        self.env_material_index = number - 1
        url = "Material\\Materials\\lightprobe.xml\\0"
        file.write(self.pack_material(url))

    # def export_materials(self, file):
    #     '''test
    #     '''
    #     number = 1
    #     file.write(self.pack_entity_header(number))

    #     url = "Material\\Materials\\default.xml\\0"
    #     file.write(self.pack_material(url))

    def export_models(self, file):

        number = len(bpy.data.meshes)

        file.write(self.pack_entity_header(number))

        index = 0
        for mesh in bpy.data.meshes:
            # todo: export material to xml files
            file.write(self.pack_model(mesh.name, index))
            index += 1

    def export_skeletions(self, file):

        number = 0
        file.write(self.pack_entity_header(number))

    def export_animations(self, file):

        number = 0
        file.write(self.pack_entity_header(number))

    def export_blendshapes(self, file):
        number = 0
        file.write(self.pack_entity_header(number))

    def export_gameobjects(self, file):

        number = 0
        for obj in bpy.data.objects:
            if obj.type == "MESH":
                number += 1

        # add env light and direction light
        number += 2

        file.write(self.pack_entity_header(number))

        for obj in bpy.data.objects:
            if obj.type == "MESH":
                file.write(self.pack_object_entry(obj))
        # env light
        # material_index, type, intensity, radius, color, direction
        file.write(
            self.pack_light_entry(None, None, 3, 1, 50, 
                Vector((1, 1, 1)), 
                Vector((1, 0, 0))
            )
        )
        file.write(
            self.pack_light_entry(None, None, 1, 20, 50, 
                Vector((1, 1, 1)), 
                Vector((1, -1, 0))
            )
        )


    def export(self, scene_name):

        with open("F:\\proj\\h3dPack\\Debug\\%s.level" % (scene_name,), "wb+") as file:
            self.export_meshes(file, scene_name)
            self.export_materials(file)
            self.export_models(file)
            self.export_skeletions(file)
            self.export_animations(file)
            self.export_blendshapes(file)
            self.export_gameobjects(file)


def export_scene():
    
    s = Scene()
    s.export("sun_temple")



if __name__ == '__main__':
    export_scene()
