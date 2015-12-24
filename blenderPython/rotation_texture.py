import bpy
import math
import bmesh

def cylinder_between(x1, y1, z1, x2, y2, z2, r):
  dx = x2 - x1
  dy = y2 - y1
  dz = z2 - z1    
  dist = math.sqrt(dx**2 + dy**2 + dz**2)

  bpy.ops.mesh.primitive_cylinder_add(
      radius = r, 
      depth = dist,
      location = (dx/2 + x1, dy/2 + y1, dz/2 + z1) 
  ) 

  phi = math.atan2(dy, dx) 
  theta = math.acos(dz/dist) 

  bpy.context.object.rotation_euler[1] = theta 
  bpy.context.object.rotation_euler[2] = phi 

def normalize(x,y,z):
    dist = math.sqrt(x**2+y**2+z**2)
    return (x/dist,y/dist,z/dist)

def dot(v1,v2):
    return (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

class MyRotationPanel(bpy.types.Panel) :
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"
    bl_context = "objectmode"
    bl_label = "My Rotation"
    
    def draw(self, context) :
        layout = self.layout
        scene = context.scene
        col = layout.column(align = True)
        col.operator("mesh.add_empty", text = "Set point ")
        col.operator("mesh.apply_rotation3", text = "See effect")
        col.operator("mesh.apply_rotation", text = "Apply rotation")
        UserPreferencesView.use_mouse_depth_navigate = True

class AddEmpty(bpy.types.Operator) :
    bl_idname = "mesh.add_empty"
    bl_label = "Add Empty"
    bl_options = {"UNDO"}

    def invoke(self, context, event) :
        sce = context.scene
        ob = context.active_object
        if (ob != None):
            ob.select = False
        if bpy.data.objects.get("Empty") != None:
            bpy.data.objects.get("Empty").select = True
            bpy.ops.object.delete()
        empty = bpy.data.objects.new("Empty", None)
        empty.empty_draw_type = 'PLAIN_AXES'
        empty.location = sce.cursor_location
        sce.objects.link(empty)
        sce.update()
        return {"FINISHED"}
    
class ApplyRotation3(bpy.types.Operator) :
    bl_idname = "mesh.apply_rotation3"
    bl_label = "See effect"
    bl_options = {"UNDO"}
    
    def invoke(self, context, event) :
        pi = 3.14159
        sce = context.scene
        ob = context.active_object
        if ob!= None:
            ob.select = True
        cursorPos = sce.cursor_location
        cursorX = cursorPos.x
        cursorY = cursorPos.y
        cursorZ = cursorPos.z
        pos1 = bpy.data.objects.get("Empty").location
        pos1x = pos1.x
        pos1y = pos1.y
        pos1z = pos1.z
        axisXc = cursorX-pos1x
        axisYc = cursorY-pos1y
        axisZc = cursorZ-pos1z
        obc_pos_x = pos1x + axisXc/2
        obc_pos_y = pos1y + axisYc/2
        obc_pos_z = pos1z + axisZc/2
        dist = math.sqrt(axisXc**2 + axisYc**2 + axisZc**2)
        # ob.location = [obc_pos_x,obc_pos_y,obc_pos_z]
        # ob.convert_space(from_space = 'WORLD', to_space = 'LOCAL')
        # phi = math.atan2(axisYc, axisXc) 
        # theta = math.acos(axisZc/dist) 
        # bpy.context.object.rotation_euler[1] = theta 
        # bpy.context.object.rotation_euler[2] = phi 
        # ob.convert_space(from_space='LOCAL', to_space='WORLD')
        if ob != None:
            ob.select = False
        if bpy.data.objects.get("Cylinder") != None:
            bpy.data.objects.get("Cylinder").select = True
            bpy.ops.object.delete()
        (axisX,axisY,axisZ) = normalize(axisXc,axisYc,axisZc)
        localX = obc_pos_x
        localY = obc_pos_y
        localZ = obc_pos_z
        # localX = ob.location.x
        # localY = ob.location.y
        # localZ = ob.location.z
        cylinder_between(localX-10*axisX,localY-10*axisY,localZ-10*axisZ,localX+10*axisX,localY+10*axisY,localZ+10*axisZ,0.01)
        bpy.data.objects.get("Cylinder").select = False

        obj = bpy.data.objects.get("StanfordBunnyTight")
        sce.objects.active = obj
        obj.select = True
        bpy.ops.object.mode_set(mode='EDIT')
        me = obj.data
        bm = bmesh.from_edit_mesh(me)

        uv_layer = bm.loops.layers.uv.verify()
        bm.faces.layers.tex.verify()  # currently blender needs both layers.

        # adjust UVs
        for f in bm.faces:
            for l in f.loops:
                luv = l[uv_layer]
                if luv.select:
                    # apply the location of the vertex as a UV
                    x = l.vert.co.x
                    y = l.vert.co.y
                    z = l.vert.co.z
                    e1 = (axisY, -axisX,0)
                    e2 = (axisX*axisZ, axisY*axisZ, -axisX*axisX-axisY*axisY)
                    t = (x-localX, y-localY, z-localZ)
                    u = dot(e1,t)
                    v = dot(e2,t)
                    luv.uv = (u/10+0.5,v/10+0.5)

        bmesh.update_edit_mesh(me)
        bpy.ops.object.mode_set(mode='OBJECT')
        if ob!= None:
            ob.select = True
            context.scene.objects.active = ob
        return {"FINISHED"}

class ApplyRotation(bpy.types.Operator) :
    bl_idname = "mesh.apply_rotation"
    bl_label = "Apply rotation"
    bl_options = {"UNDO"}
    
    def invoke(self, context, event) :
        sce = context.scene
        ob = context.active_object
        obj = sce.objects.get('StanfordBunnyTight')
        ob.select = True
        cursorPos = sce.cursor_location
        cursorX = cursorPos.x
        cursorY = cursorPos.y
        cursorZ = cursorPos.z
        pos1 = bpy.data.objects.get("Empty").location
        pos1x = pos1.x
        pos1y = pos1.y
        pos1z = pos1.z
        axisXc = cursorX-pos1x
        axisYc = cursorY-pos1y
        axisZc = cursorZ-pos1z
        obc_pos_x = pos1x + axisXc/2
        obc_pos_y = pos1y + axisYc/2
        obc_pos_z = pos1z + axisZc/2
        dist = math.sqrt(axisXc**2 + axisYc**2 + axisZc**2)
        ob.location = [obc_pos_x,obc_pos_y,obc_pos_z]
        ob.convert_space(from_space = 'WORLD', to_space = 'LOCAL')
        phi = math.atan2(axisYc, axisXc) 
        theta = math.acos(axisZc/dist) 
        bpy.context.object.rotation_euler[1] = theta 
        bpy.context.object.rotation_euler[2] = phi 
        ob.convert_space(from_space='LOCAL', to_space='WORLD')
        ob.select = False
        if bpy.data.objects.get("Cylinder") != None:
            bpy.data.objects.get("Cylinder").select = True
            bpy.ops.object.delete()
        (axisX,axisY,axisZ) = normalize(axisXc,axisYc,axisZc)
        localX = ob.location.x
        localY = ob.location.y
        localZ = ob.location.z
        cylinder_between(localX-10*axisX,localY-10*axisY,localZ-10*axisZ,localX+10*axisX,localY+10*axisY,localZ+10*axisZ,0.01)
        bpy.data.objects.get("Cylinder").select = False
        ob.select = True
        context.scene.objects.active = ob
        ob.active_material_index = 0
        bpy.ops.object.material_slot_deselect()
        boo = ob.modifiers.new('Booh', 'BOOLEAN')        
        #obj.data.materials.pop(0, update_data=True)
        obj.data.materials.clear()
        boo.object = obj
        boo.operation = 'DIFFERENCE'
        bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Booh")
        sce.objects.unlink(obj)
        bpy.ops.object.mode_set(mode='EDIT')
        return {"FINISHED"}
       
     
def register():
    bpy.utils.register_class(ApplyRotation3)
    bpy.utils.register_class(ApplyRotation)
    bpy.utils.register_class(AddEmpty)
    bpy.utils.register_class(MyRotationPanel)
    
   
def unregister():
    bpy.utils.unregister_class(MyRotationPanel)
    
if __name__ == "__main__" :
    register()   
