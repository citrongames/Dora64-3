static void circular_cameras(const Memory& seed) {
 const std::array<Function,4> helpers{dora3_s21_mini_801C578C,dora3_s21_mini_801C5908,dora3_s22_mini_801C51B0,dora3_s22_mini_801C532C};
 for(unsigned helper=0;helper<4;++helper)for(unsigned view=0;view<4;++view)
 for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u})
 for(const auto& angles:std::array<std::array<float,2>,7>{{{359,1},{1,359},{0,180},{180,0},{0,89},{0,90},{0,91}}}) {
  native_minigame_reset();Memory m=seed;const unsigned index=helper<2?0:1;arm(m,index,divider);
  auto c=context();const auto& d=dora3::minigame::definitions[index];requested=fps;
  const uint32_t cam=camera+view*0x34,body=0x80171CD0+(view+1)*0xCC;
  const bool opposite=helper%2==0;const double target=angles[1];
  m.F(cam+0x1C,angles[0]);m.F(cam+0x30,1000);m.F(body+0x10,float(std::fmod(target+(opposite?180:0),360.)));
  const double error=std::remainder(target-angles[0],360.);
  c.r4=int32_t(cam);c.r5=view;
  dora3_native_minigame_source(m.ram(),&c,d.rom,d.update,0);helpers[helper](m.ram(),&c);
  dora3_native_minigame_source(m.ram(),&c,d.rom,d.update,1);
  const double source=m.R(cam+0x1C),step=std::remainder(source-angles[0],360.);
  const double gain=std::abs(error)>90?1./30:1./15;
  // The actual Source result defines its signed180 tie.
  close(std::abs(step),std::abs(error)*gain,.0001);
  const double residual=(step<0?-1:1)*std::abs(error)*(1-gain);
  const double next_gain=std::abs(residual)>90?1./30:1./15;
  const auto completed=m.data;const auto before_commands=commands;
  Memory next=m;auto next_context=context();next_context.r4=int32_t(cam);next_context.r5=view;
  helpers[helper](next.ram(),&next_context);
  native_minigame_begin(m.ram(),clock_now,divider);const auto origin=clock_now;
  for(unsigned n=1;uint64_t(n)*60<=uint64_t(fps)*divider;++n) {
   const auto elapsed=Nanoseconds(n)*second/fps;const double h=double(elapsed)*60/(double(second)*divider);
   native_minigame_update(m.ram(),&c,origin+elapsed);
   const double expected=source+residual*(1-std::pow(1-next_gain,h));
   close(std::remainder(m.R(cam+0x1C)-expected,360.),0,.0002);
   Memory eye=m;auto math=context();math.r4=int32_t(cam);dora3_s0_8002F064(eye.ram(),&math);
   for(unsigned offset:{0xCu,0x10u,0x14u,0x28u})close(m.R(cam+offset),eye.R(cam+offset));
   assert(commands==before_commands);++slices;
  }
  native_minigame_update(m.ram(),&c,origin+Nanoseconds((uint64_t(second)*divider+59)/60));
  close(std::remainder(m.R(cam+0x1C)-next.R(cam+0x1C),360.),0,.0002);
  native_minigame_reset(m.ram());assert(m.data==completed);++cases;
 }
}
static void bomb_contact_publication(const Memory& seed) {
 for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u})
 for(float distance:{0.f,74.f,75.f,100.f})for(bool own:{false,true}) {
  native_minigame_reset();Memory m=seed;arm(m,7,divider);auto c=context();requested=fps;
  for(unsigned i=0;i<230;++i)m.W(0x80171CD0+i*0xCC+0x94)=0;
  const uint32_t explosion=0x80171CD0+10*0xCC,victim=0x80171CD0,ep=uint32_t(m.W(explosion+0x98)),vp=uint32_t(m.W(victim+0x98));
  m.W(explosion+0x94)=m.W(victim+0x94)=1;m.F(explosion+0x9C,600);m.F(explosion+0xA4,300);
  m.F(explosion,0);m.F(explosion+8,0);m.F(victim,600+distance);m.F(victim+8,300);
  m.B(ep+0xDB)=own?0:1;m.H(vp+0xD0)=3;m.H(vp+0xD8)=1;
  const auto& d=dora3::minigame::definitions[7];dora3_native_minigame_source(m.ram(),&c,d.rom,d.update,0);
  dora3_native_minigame_source(m.ram(),&c,d.rom,d.update,1);native_minigame_source_completed(m.ram(),clock_now);
  close(m.R(explosion),600);close(m.R(explosion+8),300);
  // A draw lease must restore authoritative contacts, not the pre-spawn zero.
  native_minigame_begin(m.ram(),clock_now,divider);frame_active=true;
  {MinigameFrameScope frame(m.ram());m.F(explosion,601);m.F(explosion+8,301);}
  frame_active=false;close(m.R(explosion),600);close(m.R(explosion+8),300);
  c=context();c.r4=int32_t(explosion);c.r5=int32_t(victim);c.r6=0;
  dora3_s28_mini_801C713C(m.ram(),&c);
  const bool hit=!own&&distance<75;assert(m.H(vp+0xD0)==(hit?2:3));assert(m.H(vp+0xD8)==(hit?30:1));
  // Same explosion cannot damage again during the Source invulnerability wait.
  c=context();c.r4=int32_t(explosion);c.r5=int32_t(victim);c.r6=0;dora3_s28_mini_801C713C(m.ram(),&c);
  assert(m.H(vp+0xD0)==(hit?2:3));native_minigame_reset(m.ram());++cases;
 }
}
static void source_terminal_policies(const Memory& seed) {
 // Real generated end/pause/seek wrappers; bind/resource commands remain the
 // documented opaque fixture boundary. Model render/morph is checked separately.
 for(unsigned index:{2u,3u,6u,8u})for(unsigned character=0;character<6;++character)
 for(unsigned result=0;result<2;++result)for(unsigned ended=0;ended<2;++ended) {
  native_minigame_reset();Memory m=seed;arm(m,index,3);auto c=context();requested=144;
  const uint32_t model=0x80310000,descriptors=0x80310200,policy=0x80310300;
  m.W(actor+0x74)=int32_t(model);m.W(model+0x18)=int32_t(descriptors);m.W(model+0x60)=int32_t(descriptors+0x100);
  m.F(descriptors+4,100);m.F(model+0x64,ended?1:99);m.B(model+0x7C)=ended?2:0;
  if(index==6||index==8) {
   m.H(policy+4)=result?-1:64;c.r4=int32_t(actor);c.r5=int32_t(policy);
   (index==6?dora3_s27_mini_801C45E4:dora3_s29_mini_801C4C74)(m.ram(),&c);
   close(m.R(model+0x64),ended?(result?100:64):99);assert(bool(m.B(model+0x7C)&8)==bool(ended&&result));
  } else if(index==2) {
   m.W(0x801C9228)=character;m.W(0x801C9244)=result?0:100;m.W(0x801C9250)=50;
   m.W(0x801C9240)=result?9:8;m.W(0x801C913C)=-1;
   c.r5=int32_t(actor);dora3_s23_mini_801C69CC(m.ram(),&c);
   if(!ended)close(m.R(model+0x64),99);else assert(m.R(model+0x64)!=1);
  } else {
   m.W(0x801CA3F8)=character;m.W(0x801CA464)=0;m.W(0x801CA434)=result?0:100;
   m.W(0x801CA454)=result?14:13;c.r5=int32_t(actor);
   dora3_s24_mini_801C8164(m.ram(),&c);
   if(!ended)close(m.R(model+0x64),99);else assert(m.R(model+0x64)!=1);
  }
  ++cases;
 }
}
static void bomb_actual_walk(const Memory& seed) {
 for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
  native_minigame_reset();Memory m=seed;arm(m,7,divider);auto c=context();requested=fps;
  m.W(0x801C8AC4)=2;
  for(unsigned lane=0;lane<4;++lane) {
   const uint32_t body=0x80171CD0+lane*0xCC,physical=uint32_t(m.W(body+0x98)),control=0x801C8948+lane*0x30;
   m.H(control+0x18)=0;m.H(physical+0xD0)=3;m.H(physical+0xD8)=1;m.W(physical+0x4C)=0;
   m.B(0x80171578+lane)=48;m.B(0x801AC6D4+lane)=-48;
   m.F(body+0x9C,150+lane*5);m.F(body+0xA0,0);m.F(body+0xA4,150+lane*5);
   for(unsigned axis=0;axis<3;++axis)m.F(body+4*axis,m.R(body+0x9C+4*axis));
  }
  const auto& d=dora3::minigame::definitions[7];
  dora3_native_minigame_source(m.ram(),&c,d.rom,d.update,0);
  for(unsigned lane=0;lane<4;++lane){c.r4=lane;mini8_body(m.ram(),&c);c.r4=lane;mini8_shadow(m.ram(),&c);}
  dora3_native_minigame_source(m.ram(),&c,d.rom,d.update,1);
  const auto source=m.data;const auto source_x=m.R(0x80171CD0+0x9C),source_z=m.R(0x80171CD0+0xA4);
  const auto before_commands=commands;native_minigame_begin(m.ram(),clock_now,divider);
  bool moved=false;const auto origin=clock_now;
  for(unsigned n=1;uint64_t(n)*60<uint64_t(fps)*divider;++n) {
   native_minigame_update(m.ram(),&c,origin+Nanoseconds(n)*second/fps);
   moved|=m.R(0x80171CD0+0x9C)!=source_x||m.R(0x80171CD0+0xA4)!=source_z;
   for(unsigned lane=0;lane<4;++lane) {
    const uint32_t body=0x80171CD0+lane*0xCC,shadow=0x80171CD0+(65+lane)*0xCC;
    for(unsigned axis=0;axis<3;++axis)close(m.R(body+4*axis),m.R(body+0x9C+4*axis));
    close(m.R(shadow),m.R(body));close(m.R(shadow+8),m.R(body+8));
    Memory matrix=m;auto math=context();math.r4=int32_t(body);dora3_s0_8002DA98(matrix.ram(),&math);
    for(unsigned word=0;word<16;++word)assert(m.W(body+0x24+4*word)==matrix.W(body+0x24+4*word));
    ++slices;
   }
   assert(commands==before_commands);
  }
  assert(moved);native_minigame_reset(m.ram());assert(m.data==source);++cases;
 }
}
static void source_voice_return(const Memory& seed) {
 for(bool ended:{false,true}) {
  native_minigame_reset();Memory m=seed;arm(m,2,3);auto c=context();requested=144;
  const uint32_t model=0x80310000,descriptors=0x80310200,control=0x80310300;
  m.W(actor+0x74)=int32_t(model);m.W(model+0x18)=int32_t(descriptors);m.W(model+0x60)=int32_t(descriptors+0x100);
  m.F(descriptors+4,100);m.F(model+0x64,ended?1:99);m.B(model+0x7C)=ended?2:0;
  m.W(control+0x14)=m.W(control+0x18)=3;m.W(control+0x20)=1;
  c.r4=0;c.r5=int32_t(actor);c.r6=int32_t(control);const auto before_commands=commands;
  dora3_s23_mini_801C675C(m.ram(),&c);
  assert(m.W(control+0x14)==(ended?0:3)&&m.W(control+0x18)==(ended?0:3));
  assert(m.W(control+0x20)==(ended?0:1));assert(commands-before_commands==unsigned(ended));++cases;
 }
}
static void circular_moving_targets(const Memory& seed) {
 const std::array<Function,4> helpers{dora3_s21_mini_801C578C,dora3_s21_mini_801C5908,dora3_s22_mini_801C51B0,dora3_s22_mini_801C532C};
 for(unsigned helper=0;helper<4;++helper)
 for(const auto& input:std::array<std::array<float,3>,5>{{{359,358,3},{1,2,-3},{0,159,20},{0,201,-20},{89,177,3}}}) {
  native_minigame_reset();Memory m=seed;const unsigned index=helper<2?0:1;arm(m,index,3);requested=0;
  const bool opposite=helper%2==0;const uint32_t heading=actor+0x10;const auto norm=[](double a){return a-360*std::floor(a/360);};
  const double initial_heading=norm(input[1]-(opposite?180:0)),rate=input[2],target=norm(input[1]+rate);
  m.F(camera+0x1C,input[0]);m.F(heading,float(initial_heading));m.F(camera+0x30,1000);
  MinigameEquations graph;MinigameEquations::Lifetime owner;owner.scene=4;
  graph.begin_source(m.ram(),dora3::minigame::definitions[index].rom,4,clock_now,3);
  auto old=graph.load_float(heading,float(initial_heading));auto increment=graph.constant(rate);
  graph.store_float(heading,graph.operation(MinigameEquations::Operation::Add,old,increment),float(norm(initial_heading+rate)),MinigameEquations::FieldRole::Angle,owner);
  m.F(heading,float(norm(initial_heading+rate)));auto target_expression=graph.load_float(heading,m.R(heading));
  if(opposite)target_expression=graph.operation(MinigameEquations::Operation::Add,target_expression,graph.constant(180));
  auto c=context();c.r4=int32_t(camera);c.r5=0;helpers[helper](m.ram(),&c);const auto completed=m.R(camera+0x1C);
  m.F(camera+0x1C,input[0]);const double gain=std::abs(std::remainder(target-input[0],360.))>90?1./30:1./15;
  graph.store_circular_approach(camera+0x1C,target_expression,completed,gain,owner);m.F(camera+0x1C,completed);graph.end_source(m.ram(),clock_now);
  const auto source=m.data;Memory next=m;next.F(heading,float(norm(initial_heading+2*rate)));c=context();c.r4=int32_t(camera);c.r5=0;helpers[helper](next.ram(),&c);
  for(unsigned n=0;n<=10;++n){graph.sample(m.ram(),clock_now+n*5'000'000);assert(std::isfinite(m.R(camera+0x1C)));++slices;}
  close(std::remainder(m.R(camera+0x1C)-next.R(camera+0x1C),360.),0,.0003);
  graph.restore_source(m.ram());assert(m.data==source);++cases;
 }
}
