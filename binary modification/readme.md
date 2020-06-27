File description
=================

## insert.c
- expand():
   I change the descriptor of .rsrc to RWX and expand the size for new funcs / IID,
   because there seems no more space in PE header to append the IAT or section.

- modify_IID():
  Move the IID, IAT, IIBN to .rsrc section

- insert_func():
  
  - `auto_act_proc() // 0x402360`  
  Insert recv_send_autoact_wrap() to 0x490070.
  
  At `0x401FDD`, change
```c
if(player->mode==1) 
    call auto_act_proc 
```
  to:
```c
if(palyer->mode!=0)
    call 0x490070 //recv_send_autoact_wrap() 
```
    
  
  - `rand() // 0x40A320`
  
  Copy `rand()` to `0x490100`(new_rand()) and give this a new rand seed (`0x490200`).
  
  Hook the `rand()` in `processCollisionBetweenBallAndPlayer() (0x4030D7)` to `new_rand()`, such that I can handle the sync for each user in a simpler way. (It will cause the ball bounce to different place if the rand seed is not same in each user. )
  
  - 2P setting `0x401F1A, 0x404F27`
  
  Insert `set_2p_init()` to `0x490000`.
  
  Hook `0x404F25` to:
```c
test al,4
jnz 0x490000 //set_2p_init() 
```
    
  Modify `0x401F1A` to:
```c
if mode<0 || mode>5 :
    return ; 
```
  this makes us able to set player->mode up to 2,3,4, such that I can do some condition control which is more complex. In my case, I set the player->mode  for additional control for `send_recv_wrap()`.
  

  
  - handle for game over `0x40234B`
  
  Set player->mode = 0 to exit the condition control in `send_recv_wrap()` so player can prevent being blocked by `recv()` when game over.
  
  At `0x40234B`:
```c
mov dword ptr [esi+0C0h], 0
nop 
```
  At `0x4022e0`:
```c
if player->game_over != 1:
   jmp 0x402356 ( Original: 0x40234B ) 
```
       


## func_gen.py
