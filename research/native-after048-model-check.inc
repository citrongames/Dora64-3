// Last fractional interval of a Source one-shot; real SDK pose/morph and flags.
{
 const uint32_t terminal_actor=0x80360000;W(terminal_actor+0x74)=int32_t(model);
 unsigned terminal_samples=0;
 for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u})
 for(unsigned policy=0;policy<5;++policy) {
  now+=second;seek(9);B(model+0x7C)=0;
  W(0x801A39DC)=int(divider);adapter.hook(rdram,&ctx,0x80030850,now);
  ctx.r4=int32_t(terminal_actor);const auto command_context=ctx;
  if(policy!=0)dora3_native_model_terminal_pose(rdram,&ctx,policy==2?1:0);
  assert(std::memcmp(&ctx,&command_context,sizeof(ctx))==0);
  if(policy==3)seek(9); // New run invalidates an earlier intent.
  if(policy==4)adapter.hook(rdram,&ctx,0x80030850,now); // Stale Source step.
  ctx.r4=int32_t(model);dora3_s0_800393C4(rdram,&ctx);
  ctx.r4=int32_t(model);ctx.r5=std::bit_cast<int32_t>(3.f);dora3_s0_8003948C(rdram,&ctx);
  assert(R(model+0x64)==2 && (B(model+0x7C)&2));
  const auto source_time=W(model+0x64),source_pose=W(bone+0x14);const auto source_flags=B(model+0x7C);
  for(unsigned n=0;uint64_t(n)*60<=uint64_t(fps)*divider;++n) {
   const auto elapsed=Nanoseconds(n)*second/fps;const double h=double(elapsed)*60/(double(second)*divider);
   const double position=9.+3*h;const float time=float(policy==1?std::min(position,10.):std::fmod(position,10.));
   const unsigned bank=n&1;W(0x801A9544)=int(bank);const auto other=H(banks[bank^1]);const auto context=ctx;
   adapter.begin_frame(rdram,&ctx,now+elapsed,bank);
   assert(std::abs(R(bone+0x14)-10*time)<.001f);
   assert(std::abs(int(H(banks[bank]))-int(time*10))<=1);
   assert(H(banks[bank^1])==other && W(model+0x64)==source_time && B(model+0x7C)==source_flags);
   assert(std::memcmp(&ctx,&context,sizeof(ctx))==0);
   adapter.end_frame(rdram);assert(W(bone+0x14)==source_pose);++terminal_samples;
  }
 }
 // Reverse loops remain reverse loops even if a reviewed owner was queried.
 now+=second;seek(1);B(model+0x7C)=0;W(0x801A39DC)=3;adapter.hook(rdram,&ctx,0x80030850,now);
 ctx.r4=int32_t(terminal_actor);dora3_native_model_terminal_pose(rdram,&ctx,0);
 ctx.r4=int32_t(model);dora3_s0_800393C4(rdram,&ctx);
 ctx.r4=int32_t(model);ctx.r5=std::bit_cast<int32_t>(-2.f);dora3_s0_8003948C(rdram,&ctx);
 W(0x801A9544)=0;adapter.begin_frame(rdram,&ctx,now+50'000'000,0);
 assert(std::abs(R(bone+0x14)-90.f)<.001f);adapter.end_frame(rdram);
 std::cout<<"PASS: "<<terminal_samples<<" one-shot terminal pose/morph samples, ordinary loops, component/run/step guards, reverse control, Source flags/time/context and bank restoration.\n";
 seek(0);B(model+0x7C)=0;
}
