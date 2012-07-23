//<iCost>64</iCost> 
//<Era>ERA_ANCIENT</Era>
//<Era>ERA_CLASSICAL</Era>
//<Era>ERA_MEDIEVAL</Era>
//<Era>ERA_RENAISSANCE</Era>
//<Era>ERA_INDUSTRIAL</Era>
//<Era>ERA_MODERN</Era>
//<Era>ERA_TRANSHUMAN</Era>
//<Era>ERA_FUTURE</Era>

var techs = input.TechInfos.TechInfo;
for each (var tech in techs) {  
  var cost = tech.iCost;
  var era = tech.Era.toString();
  switch (era) {
    case "ERA_ANCIENT": break;
    case "ERA_CLASSICAL": break;
    case "ERA_MEDIEVAL": break;
    case "ERA_RENAISSANCE": break;
    case "ERA_INDUSTRIAL": break;
    case "ERA_MODERN": break;
    case "ERA_TRANSHUMAN": break;
    case "ERA_FUTURE":
      tech.iCost = cost * 2; 
      break;   
  } 
  alert(tech.Type + " cost " + tech.iCost);
  break; 
}  


                 