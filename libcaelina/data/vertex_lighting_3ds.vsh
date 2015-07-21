.alias plz_no_crash c95 as (0.0, 4.0, 1.0, 8.0)
.alias negative_one c94 as (-1.0, -1.0, -1.0, -1.0)
.alias zero         c93 as (0.0, 0.0, 0.0, 0.0)
.alias half_tau     c92 as (180.0, 180.0, 0.0, 0.0)
.alias one          c91 as (1.0, 1.0, 1.0, 1.0)

.alias outpos  o0      as position
.alias outcol  o1      as color
.alias outtex0 o2.xyzw as texcoord0

.alias projection c0
.alias modelview  c4
.alias normal_mtx c8

//light 0
.alias light0_ambient       c16
.alias light0_diffuse       c17
.alias light0_specular      c18
.alias light0_position      c19
.alias light0_spotdir       c20
.alias light0_spot_cutoff   c21 //x = cutoff (degrees) y = cos(cutoff), z = spot_exponent
.alias light0_attenuation   c22 //x = constant, y = linear, z = quad


.alias material_ambient     c40
.alias material_diffuse     c41
.alias material_specular    c42
.alias material_emissive    c43
.alias material_shininess   c44 // .x = value

.alias light_model_ambient  c50

// it's expected that the program submits (0.0, 0.0, 1.0, <enabling float>)
// the enabling float should be 0.0 == non-local, > 0.0 local
.alias local_viewer c89


.alias enable_light0        b0
.alias enable_light1        b1
.alias enable_light2        b2
.alias enable_light3        b3
.alias enable_light4        b4
.alias enable_light5        b5
.alias enable_light6        b6
.alias enable_light7        b7

.alias vertex      v0
.alias v_texcoord  v1
.alias v_color     v2
.alias v_normal    v3

// holds data to loop pow func 128 times (enough for spotlight cutoff)
.alias pow128_maxloop  i0 as (128, 0, 1, 0)


main:
    // temp.pos = mdlView * in.pos
    dp4 r9.x, modelview[0], vertex
    dp4 r9.y, modelview[1], vertex
    dp4 r9.z, modelview[2], vertex
    dp4 r9.w, modelview[3], vertex

    dp3 r0.x, normal_mtx[0].xyz, v_normal.xyz
    dp3 r0.y, normal_mtx[1].xyz, v_normal.xyz
    dp3 r0.z, normal_mtx[2].xyz, v_normal.xyz
    call normalize until end_normalize
    mov r8, r0
    // result.pos = prjMtx * temp.pos
    dp4 outpos.x, projection[0], r9
    dp4 outpos.y, projection[1], r9
    dp4 outpos.z, projection[2], r9
    dp4 outpos.w, projection[3], r9
    // result.texcoord = in.texcoord
    mov outtex0, v_texcoord
    // result.color = in.color

    // do color
    mov r6, material_emissive
    mov r0, material_ambient
    mul r0, r0, light_model_ambient
    add r6, r6, r0
    if enable_light0
        // do diffuse
        mov r0, light0_position
        mov r1, r9
        mul r1, r1, negative_one
        add r0, r0, r1
        call normalize until end_normalize
        mov r7, r0
        // dot(L, n)
        dp3 r0.xyz, r7.xyz, r8.xyz
        max r0.xyz, r0.xyz, zero.xyz
        mul r0, r0, light0_diffuse
        mul r0, r0, material_diffuse

        // do ambient
        mov r5, r0
        mov r0, material_ambient
        mul r0, r0, light0_ambient
        add r5, r5, r0

        // do specular
        mov r0, local_viewer
        cmp r0.wx, zero.xy, >, >
        if cc.x // local_viewer value > zero
            mul r0, r9, negative_one
            call normalize until end_normalize
        endif
        dp3 r1.xyz, r7.xyz, r8.xyz
        cmp r1.xy, zero.xy, >, >
        if cc.x
            add r0, r7, r0 // s
            call normalize until end_normalize
            dp3 r0.xyz, r0.xyz, r8.xyz
            max r0.xyz, r0.xyz, zero.xyz
            // pow(max(dot(s, n), 0.0), material_shininess)
            mov r1, material_shininess.xxxx
            call pow until end_pow
            mul r0, r0, light0_specular
            mul r0, r0, material_specular
            add r5, r5, r0
        endif

        // attenuation
        mov r0, light0_position
        mov r1, r9
        mul r1, r1, negative_one
        add r0, r0, r1
        call length until end_length
        mov r1, light0_attenuation
        mul r0.y, r0.y, r0.y
        mul r1.z, r1.z, r0.y
        mul r1.y, r1.y, r0.x
        add r1.x, r1.x, r1.y
        add r1.x, r1.x, r1.z
        rcp r1.xyz, r1.xxx
        mul r5, r5, r1

        //spot light effect
        mov r0, light0_spot_cutoff
        cmp r0.xy, half_tau.xy, ==, ==
        if cc.x // = 180.0
            mov r0, one
        else
            mov r0, light0_position
            mov r1, r9
            mul r1, r1, negative_one
            add r0, r0, r1
            call length until end_length
            dp3 r0.xyz, r0.xyz, light0_spotdir.xyz
            max r0.xyzw, r0.xyzz, zero.xyzz
            cmp r0.xy, light0_spot_cutoff.yz, <, <
            if cc.x //outside cone of illumination
                mov r0, zero
            else
                mov r1, light0_spot_cutoff.zzzz
                call pow until end_pow
            endif
        endif
        mul r5, r5, r0

        add r6, r6, r5
    endif
    mov outcol, r6
    nop
    end
endmain:

// r0 = data, r1.x = pow
pow:
    log r15, r0
    mul r15, r15, r1.xxxx
    exp r0, r15
    nop
end_pow:

length:
    mov r15, r0
    mul r15.xyz, r15.xyz, r15.xyz
    add r15.x, r15.x, r15.y
    add r15.x, r15.x, r15.z
    rsq r15.xyz, r15.xxx
    rcp r0.xyz, r15.xyz
    nop
end_length:

normalize:
    mov r15, r0
    mul r15.xyz, r15.xyz, r15.xyz
    add r15.x, r15.x, r15.y
    add r15.x, r15.x, r15.z
    rsq r15.xyz, r15.xxx
    mul r0.xyz, r0.xyz, r15.xyz
    nop
end_normalize:
