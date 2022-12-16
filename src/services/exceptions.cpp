#include "services/exceptions.hpp"

namespace j::services {
  const tags::tag_definition<strings::string> err_service_name{"Service name"};
  const tags::tag_definition<strings::string> err_service_instance_name{"Service instance"};
  const tags::tag_definition<type_id::type_id> err_service_type{"Service type"};
  const tags::tag_definition<strings::string> err_interface_name{"Interface name"};
  const tags::tag_definition<type_id::type_id> err_interface_type{"Interface type"};
  const tags::tag_definition<strings::string> err_setting_name{"Setting name"};
}
