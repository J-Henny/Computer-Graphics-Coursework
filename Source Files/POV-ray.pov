#include "colors.inc"
#include "stones.inc"

#include "textures.inc"
#include "shapes.inc"
#include "glass.inc"
#include "metals.inc"
#include "woods.inc"   

background {color Black}

camera{
	location <0, 2, -3>
	look_at <0, 2, 4>
}    
plane {
    y, 0
    texture {
        pigment {
            checker
            color rgb<1.0, 0.0, 1.0>
            color rgb<0.0, 0.0, 0.0>
        }
        finish {
            diffuse 0.4
            ambient 0.2
            phong 1
            phong_size 100
            reflection 0.25
        }
    }
}
#declare mirror1=plane {
    <1, 0, 0>, 2
    texture {
        pigment {
            checker
            color rgb<1.0, 0.0, 1.0>
            color rgb<0.0, 0.0, 0.0>
        }   
        finish {
            diffuse 0.4
            ambient 0.2
            phong 1
            phong_size 100
            reflection 0.25
        }
    }
};   
#declare mirror2=plane {
    <1, 0, 0>, -2
    texture {
        pigment {
            checker
            color rgb<1.0, 0.0, 1.0>
            color rgb<0.0, 0.0, 0.0>
        }   
        finish {
            diffuse 0.4
            ambient 0.2
            phong 1
            phong_size 100
            reflection 0.25
        }
    }
};       
#declare mirror3=plane {
    <0, 1, 0>, 5
    texture {
        pigment {
            checker
            color rgb<1.0, 0.0, 1.0>
            color rgb<0.0, 0.0, 0.0>
        }   
        finish {
            diffuse 0.4
            ambient 0.2
            phong 1
            phong_size 100
            reflection 0.25
        }
    }
};  

mirror1   
mirror2     
mirror3

sphere {
	<0, 2, 5>, 2
	texture {NBglass}                 
	interior { ior 1.5}
}         

light_source { <2, 4, -3>, color Blue}


