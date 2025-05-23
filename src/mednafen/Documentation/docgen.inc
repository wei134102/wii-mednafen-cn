<?php

$mednafen_version = "0.9.16-WIP";
$mod_list = array();

$sections = array();

$curmod = "";
$cursection = "";

$ps_string = "";
$toc_string = "";
$settings_g = array();
$section_depth = 0;
$section_endhtml = '';

function GetMTime()
{
 $sf_mt = filemtime($_SERVER["SCRIPT_FILENAME"]);
 $sd_mt = filemtime("settings.def");
 $dg_mt = filemtime("docgen.inc");

 $mt = $sf_mt;

 if($sd_mt > $mt)
  $mt = $sd_mt;

 if($dg_mt > $mt)
  $mt = $dg_mt;

 return($mt);
}

function ReadModules()
{
 global $mod_list;

 $fp = fopen("modules.def", "rb");

 while(!feof($fp))
 {
  $mod = array();

  $mod['name'] = trim(fgets($fp));
  if($mod['name'] == "")
   break;

  $mod['fullname'] = trim(fgets($fp));
  $mod['nominal_width'] = (int)fgets($fp);
  $mod['nominal_height'] = (int)fgets($fp);  

  $mod_list[$mod['name']] = $mod;
 }

 fclose($fp);
}

function ReadSettings()
{
 global $settings_g;

 $settings_g = array();

 $fp = fopen("settings.def", "rb");

 while(!feof($fp))
 {
  $setting = array();

  $setting['name'] = trim(fgets($fp));
  $setting['flags'] = trim(fgets($fp));
  $setting['description'] = stripcslashes(trim(fgets($fp)));
  $setting['description_extra'] = stripcslashes(trim(fgets($fp)));
  $setting['type'] = trim(fgets($fp));
  $setting['default_value'] = trim(fgets($fp));
  $setting['minimum'] = trim(fgets($fp));
  $setting['maximum'] = trim(fgets($fp));

  $enum_count = (int)fgets($fp);
  $enum_list = array();

  while($enum_count--)
  {
   $ele = array();

   $ele['string'] = trim(fgets($fp));
   $ele['description'] = stripcslashes(trim(fgets($fp)));
   $ele['description_extra'] = stripcslashes(trim(fgets($fp)));

   array_push($enum_list, $ele);
  }

  $setting['enum_list'] = $enum_list;

  if($setting['name'] != "")
   $settings_g[$setting['name']] = $setting;
  //array_push($settings_g, $setting);
 }


 fclose($fp);
}


function BeginPage($module_name, $module_description)
{
 global $mednafen_version, $curmod, $cursection, $ps_string, $toc_string, $section_depth;

 ReadSettings();
 ReadModules();

 $curmod = $module_name;

 $section_depth++;

 $ps_string = '
<html>
 <head>
  <title>Mednafen ' . htmlspecialchars($module_description) . ' Documentation</title>
  <link rel="stylesheet" type="text/css" media="screen, projection" href="mednafen.css" />
 </head>
 <body>
  <center><h1><img alt="Mednafen" src="mednafen.png" width="696" height="204" /></h1></center>
  <center><h1>' . htmlspecialchars($module_description) . ' Documentation</h1></center>
  <center><i>Last updated ' . strftime("%B %e, %Y", GetMTime()) . '<br />Valid as of ' . htmlspecialchars($mednafen_version) . '</i></center>';


 $toc_string = '
 <p>
 <b>Table of Contents:</b>
 <ul>';

 ob_start();
}


function EndPage()
{
 global $mednafen_version, $curmod, $cursection, $ps_string, $toc_string, $section_depth;

 $toc_string .= '</ul></p><hr width="100%" />';


 $end_string = ' </body>
</html>';


 $section_string = ob_get_contents();
 ob_end_clean();


 echo($ps_string);
 echo($toc_string);

 echo($section_string);
 echo($end_string);

 $section_depth--;
}


function BeginSection($name, $nonlink_text = "", $link = FALSE, $header_link = FALSE)
{
 global $mednafen_version, $curmod, $cursection, $ps_string, $toc_string, $section_depth, $sections, $section_endhtml;

 array_push($sections, $name);
 $section_depth++;

 if($link !== FALSE)
 {
  if($link == "")
   $toc_string .= '<li />' . htmlspecialchars($name . $nonlink_text) . '<ul>';
  else
   $toc_string .= '<li /><a href="' . urlencode($link) . '">' . htmlspecialchars($name) . '</a>' . htmlspecialchars($nonlink_text) . '<ul>';

  $section_endhtml = '';
 }
 else
 {
  $aname = "";

  for($i = 0; $i < count($sections); $i++)
  {
   if($aname != "")
    $aname .= "\1";

   $aname .= $sections[$i];
  }

  $toc_string .= '<li /><a href="#' . urlencode($aname) . '">' . htmlspecialchars($name) . '</a>' . htmlspecialchars($nonlink_text) . '<ul>';

  echo('<a name="' . urlencode($aname) . '"><h' . $section_depth . '>');

  if($header_link !== FALSE)
   echo('<a href="' . $header_link . '">');

  echo(htmlspecialchars($name));

  if($header_link !== FALSE)
   echo('</a>');

  echo('</h' . $section_depth . '></a><p>');

  $section_endhtml = '</p><hr width="' . (100 - ($section_depth - 2) * 25) . '%">';
 }
}


function EndSection()
{
 global $mednafen_version, $curmod, $cursection, $ps_string, $toc_string, $section_depth, $sections, $section_endhtml;

 $toc_string .= '</ul>';

 echo($section_endhtml);

 array_pop($sections);
 $section_depth--;
}


function spis($haystack, $needle)
{
 if(strpos($haystack, $needle) === FALSE)
  return(0);
 return(1);
}

function PrintSettings($section_name = "Settings Reference")
{
 global $mednafen_version, $curmod, $cursection, $ps_string, $toc_string, $section_string, $settings_g, $mod_list;

 $de_search = array("\n",	 "fname_format.txt" );
 $de_replace = array("<br />\n", '<a href="fname_format.txt">fname_format.txt</a>' );


 $magic_settings = array();
 $magic_settings_temp = array();
 $magic_moo = array();

 BeginSection($section_name);	//'Settings Reference');
?>
<p>
<b>Caution:</b> Any settings that change emulation behavior as seen from the emulated game are shown with the setting name bolded in the table. One should be conscious of these settings if one has changed them from the default values when using network play or movies, as they can cause desychronizations if not set the same among hosts(or between the recording emulator and the playing emulator). 
</p>
<?php
 $value_types = array();
 $value_types['MDFNST_INT'] = "integer";
 $value_types['MDFNST_UINT'] = "integer";
 $value_types['MDFNST_BOOL'] = "boolean";
 $value_types['MDFNST_FLOAT'] = "real";
 $value_types['MDFNST_STRING'] = "string";
 $value_types['MDFNST_ENUM'] = "enum";

for($display_ct = 0; $display_ct < 2; $display_ct++)
{
 echo('<p><table border><tr class="TableHeader"><th>Setting:</th><th>Value Type:</th><th>Possible Values:</th><th>Default Value:</th><th>Description:</th></tr>');

 $settings = $settings_g;

 if($curmod == "" && $display_ct == 1)
  $settings = $magic_settings;

 $row_ff = 0;
 foreach($settings as $setting)
 {
  $name = $setting['name'];
  $description = $setting['description'];
  $description_extra = str_replace($de_search, $de_replace, $setting['description_extra']);
  $type = $setting['type'];
  $flags = $setting['flags'];

  if($curmod == "")
  {
   $should_continue = 0;
   foreach($mod_list as $mod=>$mod_def)
   {
    if(!strncmp($setting['name'], $mod . '.', strlen($mod) + 1))
    {
     $new_setting = $setting;

     $new_name = "&lt;system&gt;." . substr($setting['name'], strlen($mod) + 1);
     $new_setting['name'] = $new_name;

     if(isset($magic_settings_temp[$new_name]))
     {
      $m = &$magic_settings_temp[$new_name];

      if($new_setting['type'] != $m['type'])
       $m['type'] = "<i>(variable)</i>";

      if($new_setting['default_value'] != $m['default_value'])
       $m['default_value'] = "<i>(variable)</i>";

      if($new_setting['minimum'] != $m['minimum'])
       $m['minimum'] = "<i>(variable)</i>";

      if($new_setting['maximum'] != $m['maximum'])
       $m['maximum'] = "<i>(variable)</i>";

      $magic_settings[$new_name] = $m;
     }
     else
      $magic_settings_temp[$new_name] = $new_setting;

     if(!isset($magic_moo[$new_name]))
      $magic_moo[$new_name] = array();

     array_push($magic_moo[$new_name], $mod);


     $should_continue = 1;
     break;
    }

   }
   if($should_continue)
    continue;
  }
  else
  {
   if(strncmp($setting['name'], $curmod . '.', strlen($curmod) + 1))
    continue;
  }

  if(strpos($flags, "MDFNSF_SUPPRESS_DOC") !== FALSE)
   continue;

  if($display_ct != spis($flags, "MDFNSF_COMMON_TEMPLATE"))
   continue;

  $magiccowpower = "";

  if(isset($magic_moo[$name]) && sizeof($magic_moo[$name]) != sizeof($mod_list))
  {
   $magiccowpower = "<br /><ul>";
   foreach($magic_moo[$name] as $moomod)
   {
    $magiccowpower .= "<li>" . $moomod . "</li>";
   }
   $magiccowpower .= "</ul>";
  }

  printf("<tr class=\"%s\"><td class=\"ColA\"><a name=\"%s\">%s%s%s</a></td><td class=\"ColB\">%s</td><td class=\"ColC\">", 
	($row_ff ? "RowB" : "RowA"), $name, (spis($flags, "MDFNSF_EMU_STATE") ? "<b>" : ""), $name . $magiccowpower,
                                        (spis($flags, "MDFNSF_EMU_STATE") ? "</b>" : ""),
                                        $value_types[$type]);



  switch($type)
  {
   case 'MDFNST_INT':
   case 'MDFNST_UINT':
   case 'MDFNST_FLOAT': printf("%s <i>through</i> %s", $setting['minimum'], $setting['maximum']);
                      break;

   case 'MDFNST_BOOL': printf("0<br>1");
                     break;

   case 'MDFNST_ENUM':
                    {
                     $enum_list = $setting['enum_list'];
		     $pending_br = '';

		     foreach($enum_list as $ele)
                     {
                      if($ele['description'] == '')
                       continue;

		      echo($pending_br);
                      printf("%s", $ele['string']);
		      $pending_br = '<br />';
                     }
                    }
                    break;
   case 'MDFNST_STRING': printf("&nbsp;");
                       break;
  }

  $default_value = $setting['default_value'];

  // HACK
  if($setting['name'] == "sound.buffer_time")
   $default_value .= " (52 on MS Windows)";

  printf("</td><td class=\"ColD\">%s</td><td class=\"ColE\">%s<p>%s</p>", $default_value,
                                        $description, $description_extra);

  if($type == 'MDFNST_ENUM')
  {
   printf("<ul>");

   $enum_list = $setting['enum_list'];
   $pending_br = '';
   foreach($enum_list as $ele)
   {
    if($ele['description'] == '')
     continue;

    echo($pending_br);
    printf("<li><b>%s</b> - %s<br>%s</li>", $ele['string'], $ele['description'], str_replace($de_search, $de_replace, $ele['description_extra']));

    $pending_br = '<br />';
   }

   printf("</ul>");
  }


  printf("</td></tr>");

  printf("<tr><td class=\"RowSpacer\" colspan=\"5\">&nbsp</td></tr>");

  printf("\n");



  $row_ff = !$row_ff;
 } 

 echo('</table></p>');
}


 EndSection();
}



function DoModDocLinks()
{
 global $mod_list;
 
 BeginSection("Emulation Module Documentation", "", "");

 $doctor_sort = array();

 foreach($mod_list as $name=>$mod_def)
  $doctor_sort[$mod_def['name']] = $mod_def['fullname'];

 asort($doctor_sort);

 foreach($doctor_sort as $name=>$fullname)
 {
  $mod_def = $mod_list[$name];

  if($name == "cdplay" || $name == "sms" || $name == "gg")
   continue;

  BeginSection($mod_def['fullname'], " [" . $name . "]", $name . ".html");
  EndSection();
 }

 EndSection();
}

?>
