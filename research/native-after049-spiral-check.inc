// Real ROM0A46 GIMM: coupled angle plus X/Y scale recycle through480..482.
{
 std::ifstream file(argv[1],std::ios::binary);assert(file);
 std::vector<char> data((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
 assert(data.size()>32&&data.size()<0x10000);
 constexpr uint32_t real_clip=0x80370000,real_mesh=0x80340000;
 std::ifstream mesh_file(argv[2],std::ios::binary);assert(mesh_file);
 std::vector<char> mesh_data((std::istreambuf_iterator<char>(mesh_file)),std::istreambuf_iterator<char>());
 assert(mesh_data.size()<0x20000);for(unsigned i=0;i<mesh_data.size();++i)B(real_mesh+i)=int8_t(mesh_data[i]);
 const auto saved_model=model;ctx.r4=int32_t(real_mesh);dora3_s0_8003B560(rdram,&ctx);model=uint32_t(ctx.r2);
 const auto find_bone=[&](unsigned target){std::vector<uint32_t> todo{uint32_t(W(model+0x5C))};while(!todo.empty()){const auto at=todo.back();todo.pop_back();if(unsigned(W(at+4))==target)return at;for(int i=0;i<W(at+0x10);++i)todo.push_back(uint32_t(W(uint32_t(W(at+0xC))+4*i)));}std::abort();};
 const auto bone=find_bone(2);ctx.r4=int32_t(model);dora3_s0_core_80038BC8(rdram,&ctx);
 for(unsigned i=0;i<data.size();++i)B(real_clip+i)=int8_t(data[i]);
 const auto transforms=[&] {std::array<int32_t,9> result{};for(unsigned i=0;i<9;++i)result[i]=W(bone+0x14+4*i);return result;};
 
 ctx.r4=int32_t(model);ctx.r5=int32_t(real_clip);dora3_s0_8003B6BC(rdram,&ctx);
 assert(W(model+0x2C)==4);
 const auto descriptors=uint32_t(W(model+0x18)),curves=uint32_t(W(model+0x1C));
 std::array<uint32_t,3> reset_keys{};
 for(unsigned i=0;i<3;++i){const auto descriptor=descriptors+28*(i+1),curve=curves+16*unsigned(W(descriptor+0x18));assert(W(curve)==0&&W(curve+8)==4);reset_keys[i]=real_clip+uint32_t(W(curve+12));}
 assert(W(descriptors+28)==5&&W(descriptors+28+0x10)==2&&W(descriptors+56)==6&&W(descriptors+84)==6);
 const auto scale_keys=reset_keys[1];const float after=R(scale_keys+20),rate=(R(scale_keys+28)-after)/(R(scale_keys+24)-R(scale_keys+16));
 assert(std::abs(R(scale_keys+12)-.375f)<1e-6f&&R(scale_keys+8)==480.f&&R(scale_keys+16)==482.f);
 seek(481);ctx.r4=int32_t(model);dora3_s0_800393C4(rdram,&ctx);
 assert(std::abs(R(bone+0x2C)-(R(scale_keys+12)+after)*.5f)<1e-6f);
 assert(std::abs(R(bone+0x30)-(R(scale_keys+12)+after)*.5f)<1e-6f);
 {SimulationScope simulation({1000000000ull,3000000000ull,0,0});ctx.r4=int32_t(model);dora3_s0_800393C4(rdram,&ctx);
  assert(std::abs(R(bone+0x2C)-(after-rate))<1e-6f&&std::abs(R(bone+0x30)-(after-rate))<1e-6f);
 }
 unsigned coupled_samples=0;
 for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}){
  now+=second;seek(479);B(model+0x7C)=0;step(3,divider);
  const auto source_transform=transforms();const auto source_time=W(model+0x64);const auto source_flags=B(model+0x7C);
  for(unsigned n=0;uint64_t(n)*60<=uint64_t(fps)*divider;++n){
   const auto elapsed=Nanoseconds(n)*second/fps;const float time=float(479.+3.*double(elapsed)*60/(double(second)*divider));
   const unsigned bank=n&1;W(0x801A9544)=int(bank);const auto saved=ctx;
   adapter.begin_frame(rdram,&ctx,now+elapsed,bank);
   const float expected=time<=480?R(scale_keys+4)+(R(scale_keys+12)-R(scale_keys+4))*time/480.f:after+rate*(time-482.f);
   if(std::abs(R(bone+0x2C)-expected)>=2e-6f||std::abs(R(bone+0x30)-expected)>=2e-6f){std::cerr<<"spiral scale fps="<<fps<<" frame="<<n<<" t="<<time<<" x="<<R(bone+0x2C)<<" y="<<R(bone+0x30)<<" expected="<<expected<<"\n";std::abort();}
   const float angle=time<=480?360.f*time/480.f:time<482?360.f+R(reset_keys[0]+20)*(time-480.f)/2.f:R(reset_keys[0]+20)+(R(reset_keys[0]+28)-R(reset_keys[0]+20))*(time-482.f)/(960.f-482.f);
   const auto wrapped=[](float value){const float result=std::fmod(value,360.f);return result<0?result+360.f:result;};
   const float difference=std::abs(wrapped(R(bone+0x28))-wrapped(angle));
   if(std::min(difference,360.f-difference)>=.001f){std::cerr<<"actual spiral angle fps="<<fps<<" frame="<<n<<" t="<<time<<" shown="<<R(bone+0x28)<<" expected="<<angle<<"\n";std::abort();}
   assert(W(model+0x64)==source_time&&B(model+0x7C)==source_flags&&std::memcmp(&ctx,&saved,sizeof(ctx))==0);
   adapter.end_frame(rdram);assert(transforms()==source_transform);
   for(unsigned i=0;i<data.size();++i){assert(uint8_t(B(real_clip+i))==uint8_t(data[i]));}
   ++coupled_samples;
  }
 }
 ctx.r4=int32_t(model);dora3_s0_8003B77C(rdram,&ctx);model=saved_model;seek(0);B(model+0x7C)=0;
 std::cout<<"PASS: "<<coupled_samples<<" real ROM0A46 coupled angle/scale recycle samples at both Source rates/60..360 FPS, actual SDK pose/matrix, exact Original midpoint retained, Native short seam continued, unchanged clip/time/flags/context and both bank restoration.\n";
}
