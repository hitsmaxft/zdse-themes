document.addEventListener('dte-preview-ready',()=>{
  const preview=window.dtePreview,api=preview.api;
  const bindToggle=(id,setter,getter)=>{
    const element=document.getElementById(id);
    const refresh=()=>element.classList.toggle('active',!!getter());
    element.onclick=()=>{setter(!getter());preview.drawAt(preview.time());refresh();};
    refresh();
  };
  document.getElementById('radar-speed').onchange=e=>{api.zdse_radar_set_speed_profile_at(+e.target.value,Math.round(preview.time()));preview.drawAt(preview.time());};
  document.getElementById('radar-start').onchange=e=>{api.zdse_radar_set_cold_start(+e.target.value,Math.round(preview.time()));preview.drawAt(preview.time());};
  bindToggle('radar-aurora',api.zdse_radar_set_aurora,api.zdse_radar_get_aurora);
  bindToggle('radar-targets',api.zdse_radar_set_targets,api.zdse_radar_get_targets);
  bindToggle('radar-damage',api.zdse_radar_set_damage_overlay,api.zdse_radar_get_damage_overlay);
});
