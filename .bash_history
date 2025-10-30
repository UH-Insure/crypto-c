cd verification
make aes.bc
make all
make
pwd
ls ..
ls ../src/primitives
ls wrappers
ls ..
cd
cd verification/saw
cd 
cd verification
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
export CRYPTOLPATH=/work/verification/specs/cryptol_specs
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
print "=== AES128 Proof ===";
bc = llvm_load_module "/work/verification/build/aes.bc";
cry = cryptol_load "/work/verification/specs/aes128.cry";
aes_spec = cryptol_extract cry "AES128Encrypt";
key = llvm_alloc (llvm_array 16 llvm_i8);
pt  = llvm_alloc (llvm_array 16 llvm_i8);
ctx = llvm_alloc (llvm_array 11 (llvm_array 16 llvm_i8));
llvm_call bc "aes128_set_key" [ctx, key];
out_c = llvm_call bc "aes128_encrypt_block" [ctx, pt];
key_bytes = llvm_array_load key;
pt_bytes  = llvm_array_load pt;
key_bits = concat (map (zero_extend 8) key_bytes);
pt_bits  = concat (map (zero_extend 8) pt_bytes);
out_spec = aes_spec key_bits pt_bits;
llvm_verify bc "aes128_encrypt_block" [] false (\_ -> true) (\_ -> true) (prove_z3 []);
llvm_verify bc "aes128_encrypt_block" [] false (\_ -> true) (\_ -> true) (prove_z3 []);
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/test.saw
saw saw/test.saw
saw saw/aes128_proof.saw
saw saw/test.saw
saw saw/test.saw
saw saw/test.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
ls /work/src
cd /work/verification
make clean
make build/aes.bc
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
cat /saw/aes128_proof.saw
ls
cat /saw/aes128_proof.saw
cat -A saw/aes128_proof.saw | nl
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
cat -A aes128_proof.saw | head -3
cd saw
cat -A aes128_proof.saw | head -3
hmm@63417cba655d:~/verification/saw$ cat -A aes128_proof.saw | head -3
main : TopLevel ()$
main = do$
  print "=== AES128 Proof ==="$
hmm@63417cba655d:~/verification/saw$ 
which saw
saw saw/aes128_proof.saw
saw saw/aes128_proof.saw
cd ../
saw saw/aes128_proof.saw
saw saw/test.saw
saw saw/test.saw
saw saw/test.saw
saw saw/test.saw
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
ls -l /work/verification/build/aes.bc
saw saw/aes128_proof.saw 
saw -e 'print =<< pwd;'
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
saw saw/aes128_proof.saw 
cd /work/verification/specs
ln -s cryptol_specs Primitive
saw 
