from pwn import *

context.arch = "i386"
# context.os = "windows"
l = asm("""
        jnz 0x490000
        
        """,vma = 0x404f27)
l = asm("""
        cmp     dword ptr [esi+0xA4], 0
        jz     lab
        lab:
        nop

        
        """,vma=0x401fdd)
l = ["\\" + hex(ord(c))[1:] for c in l]
print(''.join(l[:10]))



print(
        
        )
# func payloads

# act_socket is called by "JMP" in 2p_setting section, so when the func ends,
# JMP back to 
act_socket = asm("""
       
        mov eax,[0x499010]
        call eax

        cmp eax,0 /* if ret==0*/
        je lab1
        
        cmp eax,1  /* if ret==1 */
        je lab2

        cmp eax,2 /* if ret==2 */
        je lab3


    lab1:
        mov edx,0
        mov eax,0
        jmp 0x404f41
        
    lab2:
        /*rand seed reset*/
        push 0x4
        push 0x490200  /*new rand seed*/
        mov eax,[0x499008]
        call eax 
        add esp,0x8



        mov edx,2
        mov eax,3
        jmp 0x404f41
    
    lab3:
        /*rand seed reset*/
        push 0x4
        push 0x490200  /*new rand seed*/
        mov eax,[0x499000]
        call eax 
        add esp,0x8


        mov edx,3
        mov eax,2
        jmp 0x404f41

        """,vma = 0x490000)

act_send_recv = asm("""
        /* this = ecx , is_computer = this+0xA4*/
        /* act = ebp+0xc*/
        cmp dword ptr [ecx+0xa4],1
        je 0x402360

        push ebp
        mov ebp,esp

        /*======auto test======*/
        mov eax,[ebp+0xc+0x4]
        push eax
        mov eax,[ebp+0x8+0x4]
        push eax
        mov eax,[ebp+0x4+0x4]
        push eax
        call 0x402360


        /*======auto test======*/


        push ecx
        push ebx
        push esi
        push edi
        mov ecx,esi
        /*  */
        mov eax , [ebp+0xc+0x4]
        push 0x10          /* size of act[4]*/
        push eax


        mov eax, [ecx+0xa4] /* is_compter*/
        cmp eax,2
        je recv
        cmp eax,3
        je send
        cmp eax,1
        je end
        cmp eax,0
        je end

auto:

send:
        mov eax,[0x499008]
        call eax
        add esp,0x8
        jmp end

recv:
        mov eax,[0x499000]
        call eax
        add esp,0x8
        jmp end

end:
        /* */
        pop edi
        pop esi
        pop ebx
        pop ecx
        mov esp,ebp
        pop ebp
        
        ret 0xc
        """,vma = 0x490070)

# act_send = asm("""
        # mov eax,[0x499008]
        # call eax
        # ret
        # """)
# act_recv = asm("""
        # mov eax,[0x499000]
        # call eax
        # ret
        # """)


payload = act_socket + "\x00"*(0x10-len(act_socket)%0x10)\
        + act_send_recv 
print("lem:0x%x"%(len(payload)))
print("lens:0x%x,0x%x"%(len(act_socket),len(act_send_recv)))

l1 = ((len(act_socket)+0x10-len(act_socket)%0x10))
# l2 = (len(act_send)+0x10-len(act_send)%0x10)
print("""
        act_socket:0x00,
        act_send_recv:0x%x,
        """%(l1))
with open("out",'wb') as f:
    f.write(payload)
