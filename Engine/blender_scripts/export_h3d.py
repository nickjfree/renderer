
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
        return self.position == other.position and 
            self.normal == other.normal and
            self.uv == other.uv and
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
        return struct.pack(self.format_str, -self.position.y, self.position.z, self.position.x,
            -self.normal.y, self.normal.z, self.normal.x,
            self.uv.x, self.uv.y,
            0,
            -self.tangent.y, self.tangent.z, self.tangent.x)

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
                    old_vertex = self.vertices[max_vertex_count - i]
                    if old_vertex == new_vertex:
                        vertex_id = max_vertex_count - i
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


def export_actived():

    obj = bpy.context.active_object

    me = obj.to_mesh()
    mesh_triangulate(me)

    mesh = Mesh(mesh=me)
    mesh.calc_data()
    mesh.export_h3d(path="F:\\proj\\h3dPack\\Debug\\cube.h3d")
    mesh.export_console()

if __name__ == '__main__':
    export_actived()

